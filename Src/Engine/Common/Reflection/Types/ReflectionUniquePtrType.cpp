/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionUniquePtrType.hpp"
#include "ReflectionClassType.hpp"
#include "../SerializationContext.hpp"
#include "../ReflectionTypeRegistry.hpp"
#include "../../Utils/Stream/OutputStream.hpp"
#include "../../Utils/Stream/InputStream.hpp"
#include "../../Config/ConfigInterface.hpp"
#include "../../Config/ConfigValue.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

UniquePtrType::UniquePtrType(const Type* underlyingType)
    : PointerType(underlyingType)
{ }

void* UniquePtrType::GetPointedData(const void* uniquePtrObject) const
{
    NFE_ASSERT(uniquePtrObject, "Trying to access nullptr");
    void** typedObject = BitCast<void**>(uniquePtrObject);
    return *typedObject;
}

const Type* UniquePtrType::GetPointedDataType(const void* uniquePtrObject) const
{
    NFE_ASSERT(uniquePtrObject, "Trying to access nullptr");
    const void* pointedData = GetPointedData(uniquePtrObject);

    if (pointedData)
    {
        if (mUnderlyingType->GetKind() == TypeKind::AbstractClass || mUnderlyingType->GetKind() == TypeKind::PolymorphicClass)
        {
            return BitCast<const IObject*>(pointedData)->GetDynamicType();
        }
        else
        {
            return mUnderlyingType;
        }
    }

    return nullptr;
}

void* UniquePtrType::Reset(void* uniquePtrObject, const Type* newDataType) const
{
    NFE_ASSERT(uniquePtrObject, "Trying to access nullptr");
    NFE_ASSERT(!newDataType || newDataType->IsA(mUnderlyingType), "Incompatible types");

    void** typedObject = BitCast<void**>(uniquePtrObject);
    void* pointedData = *typedObject;

    if (pointedData)
    {
        mUnderlyingType->DeleteObject(pointedData);
    }

    *typedObject = nullptr;

    void* newObject = nullptr;

    if (newDataType)
    {
        newObject = newDataType->CreateRawObject();
        *typedObject = newObject;
    }

    return newObject;
}

bool UniquePtrType::Serialize(const void* object, IConfig& config, ConfigValue& outValue, SerializationContext& context) const
{
    NFE_ASSERT(object, "Trying to serialize nullptr");
    const void* pointedData = GetPointedData(object);

    if (pointedData)
    {
        mUnderlyingType->Serialize(pointedData, config, outValue, context);
    }
    else // null pointer
    {
        outValue = ConfigValue(0);
    }

    return true;
}

bool UniquePtrType::Deserialize(void* outObject, const IConfig& config, const ConfigValue& value, const SerializationContext& context) const
{
    NFE_ASSERT(outObject, "Trying to deserialize to nullptr");

    if (value.Is<int32>()) // nullptr
    {
        if (value.Get<int32>() != 0)
        {
            NFE_LOG_WARNING("Expected zero");
        }

        Reset(outObject);
        return true;
    }
    else if (value.IsObject()) // valid object
    {
        const char* typeName = nullptr;
        const auto configIteratorCallback = [&typeName] (StringView key, const ConfigValue& value)
        {
            // marker found
            if (key == ClassType::TYPE_MARKER)
            {
                if (!value.IsString())
                {
                    NFE_LOG_ERROR("Marker type found - string expected");
                    return false;
                }

                typeName = value.Get<const char*>();
                return false;
            }

            return true;
        };

        // extract target object type from marker
        config.Iterate(configIteratorCallback, value.GetObj());

        const Type* targetType = nullptr;
        if (typeName)
        {
            // get type from name
            targetType = ITypeRegistry::GetInstance().GetExistingType(typeName);
            if (!targetType)
            {
                NFE_LOG_ERROR("Type not found: '%s'", typeName);
                return false;
            }

            if (!targetType->IsA(mUnderlyingType))
            {
                const StringView name = mUnderlyingType->GetName();
                NFE_LOG_ERROR("Target type '%s' is not related with pointed type '%.*s'", typeName, name.Length(), name.Data());
                return false;
            }
        }
        else
        {
            if (mUnderlyingType->GetKind() != TypeKind::AbstractClass)
            {
                NFE_LOG_WARNING("Type marker not found - using pointed type as a reference");
                targetType = mUnderlyingType;
            }
            else // pointed type is abstract class
            {
                NFE_LOG_ERROR("Type marker not found - cannot resolve target type");
                return false;
            }
        }

        if (!targetType->IsConstructible())
        {
            NFE_LOG_ERROR("Target type '%s' is not constructible", typeName);
            return false;
        }

        // construct the object & assign to the smart pointer
        void* pointedData = Reset(outObject, targetType);
        if (!pointedData) 
        {
            // failed to allocate memory?
            return false;
        }

        // deserialize the object
        return targetType->Deserialize(pointedData, config, value, context);
    }

    NFE_LOG_ERROR("Expected zero (nullptr) or an object");
    return false;
}

bool UniquePtrType::SerializeBinary(const void* object, OutputStream* stream, SerializationContext& context) const
{
    const Type* pointedDataType = GetPointedDataType(object);

    if (!TypeRegistry::GetInstance().SerializeTypeName(pointedDataType, stream, context))
    {
        return false;
    }

    if (pointedDataType)
    {
        const void* pointedData = GetPointedData(object);
        NFE_ASSERT(pointedData, "Pointed data is expected to be valide if type is not null");

        if (!pointedDataType->SerializeBinary(pointedData, stream, context))
        {
            return false;
        }
    }

    return true;
}

bool UniquePtrType::DeserializeBinary(void* outObject, InputStream& stream, const SerializationContext& context) const
{
    // deserialize type from stream
    const Type* serializedType = nullptr;
    TypeDeserializationResult typeDeserializationResult = TypeRegistry::GetInstance().DeserializeTypeName(serializedType, stream, context);

    if (typeDeserializationResult == TypeDeserializationResult::Error)
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return false;
    }

    // serialized type not known
    if (typeDeserializationResult == TypeDeserializationResult::UnknownType)
    {
        // TOOO skip object
        // TODO report unknown type
        NFE_LOG_WARNING("Failed to deserialize UniquePtr of type '%s': type in data is not know. Probably the type was removed from code.", GetName().Str());
        return false;
    }

    // handle nullptr
    if (nullptr == serializedType)
    {
        Reset(outObject);
        return true;
    }

    if (!serializedType->IsA(GetUnderlyingType()))
    {
        // TOOO skip object (for now)
        // TODO report pointer type mismatch
        // TODO deserialize to Config object so it can be used for migration to newer format
        NFE_LOG_WARNING("Failed to deserialize UniquePtr of type '%s': Type in data is '%s' and it's not compatible with underlying pointer type '%s'",
            GetName().Str(), serializedType->GetName().Str(), GetUnderlyingType()->GetName().Str());
        return false;
    }

    void* pointedData = Reset(outObject, serializedType);
    if (!pointedData) 
    {
        // failed to allocate memory?
        return false;
    }

    // deserialize the object
    if (!serializedType->DeserializeBinary(pointedData, stream, context))
    {
        // TODO skip object
        return false;
    }

    return true;
}

} // namespace RTTI
} // namespace NFE

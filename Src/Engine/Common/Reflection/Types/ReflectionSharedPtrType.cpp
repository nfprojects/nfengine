/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionSharedPtrType.hpp"
#include "ReflectionClassType.hpp"
#include "../SerializationContext.hpp"
#include "../ReflectionTypeRegistry.hpp"
#include "../../Config/ConfigInterface.hpp"
#include "../../Utils/Stream/OutputStream.hpp"
#include "../../Utils/Stream/InputStream.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

using ObjectType = SharedPtr<IObject>;

static const StringView OBJECT_ID_MARKER("__objID");
static const StringView TYPE_MARKER("__type");
static const StringView VALUE_MARKER("__value");

SharedPtrType::SharedPtrType(const Type* underlyingType)
    : PointerType(underlyingType)
{ }

const Common::String SharedPtrType::BuildTypeName(const Type* underlyingType)
{
    NFE_ASSERT(underlyingType, "Invalid type");
    return Common::String("SharedPtr<") + underlyingType->GetName() + '>';
}

void SharedPtrType::OnInitialize(const TypeInfo& info)
{
    NFE_UNUSED(info);

    NFE_ASSERT(mSize == sizeof(ObjectType), "Invalid SharedPtrType size");
    NFE_ASSERT(mAlignment == alignof(ObjectType), "Invalid SharedPtrType alignment");
}

void* SharedPtrType::GetPointedData(const void* ptrObject) const
{
    NFE_ASSERT(ptrObject, "Trying to access nullptr");
    const ObjectType& typedObject = *static_cast<const ObjectType*>(ptrObject);
    return typedObject.Get();
}

const Type* SharedPtrType::GetPointedDataType(const void* ptrObject) const
{
    NFE_ASSERT(ptrObject, "Trying to access nullptr");
    const ObjectType& typedObject = *static_cast<const ObjectType*>(ptrObject);

    if (typedObject)
    {
        if (mUnderlyingType->IsA(GetType<IObject>()))
        {
            return typedObject.Get()->GetDynamicType();
        }
        else
        {
            return mUnderlyingType;
        }
    }

    return nullptr;
}

void SharedPtrType::Assign(void* sharedPtrObject, const SharedPtr<IObject>& newPtr) const
{
    NFE_ASSERT(sharedPtrObject, "Trying to access nullptr");

    ObjectType& typedObject = *static_cast<ObjectType*>(sharedPtrObject);
    typedObject = newPtr;
}

void* SharedPtrType::Reset(void* ptrObject, const Type* newDataType) const
{
    NFE_ASSERT(ptrObject, "Trying to access nullptr");
    ObjectType& typedObject = *static_cast<ObjectType*>(ptrObject);
    if (newDataType)
    {
        typedObject.Reset(newDataType->CreateObject<IObject>());
    }
    else
    {
        typedObject.Reset();
    }
    return typedObject.Get();
}

bool SharedPtrType::Serialize(const void* object, IConfig& config, ConfigValue& outValue, SerializationContext& context) const
{
    NFE_ASSERT(object, "Trying to serialize nullptr");
    const void* pointedData = GetPointedData(object);
    const Type* pointedDataType = GetPointedDataType(object);

    ConfigObject root;

    // attach type marker
    if (pointedDataType && pointedData)
    {
        const ObjectPtr& typedObject = *BitCast<const ObjectPtr*>(object);
        const bool wasMapped = context.IsObjectMapped(typedObject.Get());

        // type marker
        if (!wasMapped)
        {
            ConfigValue marker(pointedDataType->GetName().Str());
            config.AddValue(root, TYPE_MARKER, marker);
        }

        // object ID 
        {
            const uint32 objectIndex = context.MapObject(typedObject);
            const ConfigValue objectIdMarker(objectIndex);
            config.AddValue(root, OBJECT_ID_MARKER, objectIdMarker);
        }

        // object itself
        if (!wasMapped)
        {
            ConfigValue pointedObjectValue;
            if (!pointedDataType->Serialize(pointedData, config, pointedObjectValue, context))
            {
                return false;
            }
            config.AddValue(root, VALUE_MARKER, pointedObjectValue);
        }
    }

    outValue = ConfigValue(root);
    return true;
}

bool SharedPtrType::Deserialize(void* outObject, const IConfig& config, const ConfigValue& value, SerializationContext& context) const
{
    NFE_ASSERT(outObject, "Trying to deserialize to nullptr");

    if (!value.IsObject())
    {
        NFE_LOG_ERROR("Expected object");
        return false;
    }

    bool errorFound = false;
    uint32 objectID = UINT32_MAX;
    const char* typeName = nullptr;
    const ConfigValue* pointedObjectValue = nullptr;
    const auto configIteratorCallback = [&](const StringView key, const ConfigValue& value)
    {
        if (key == TYPE_MARKER)
        {
            if (!value.IsString())
            {
                NFE_LOG_ERROR("Type marker found - string expected");
                errorFound = true;
                return false;
            }

            typeName = value.Get<const char*>();
        }
        else if (key == OBJECT_ID_MARKER)
        {
            if (!value.Is<int32>())
            {
                NFE_LOG_ERROR("Object ID marker found - uint32 expected");
                errorFound = true;
                return false;
            }

            objectID = static_cast<uint32>(value.Get<int32>());
        }
        else if (key == VALUE_MARKER)
        {
            pointedObjectValue = &value;
        }

        return true;
    };

    // extract target object type from marker
    config.Iterate(configIteratorCallback, value.GetObj());

    if (errorFound)
    {
        return false;
    }

    const Type* targetType = nullptr;

    if (!typeName && !pointedObjectValue && objectID == UINT32_MAX)
    {
        // handle nullptr

        Reset(outObject);

        return true;
    }
    else if (!typeName && !pointedObjectValue && objectID != UINT32_MAX)
    {
        // assign already mapped object

        SharedPtr<IObject> unmappedObject;
        if (!context.UnmapObject(objectID, unmappedObject))
        {
            NFE_LOG_ERROR("Failed to unmap object with index %u. Corrupted data?", objectID);
            return false;
        }

        Assign(outObject, unmappedObject);

        NFE_ASSERT(GetPointedData(outObject) == unmappedObject.Get(), "Assignment failed");

        return true;
    }
    else if (typeName)
    {
        if (!pointedObjectValue)
        {
            NFE_LOG_ERROR("Type marker found but value marker missing");
            return false;
        }

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
        if (mUnderlyingType->GetKind() != TypeKind::Class || !static_cast<const ClassType*>(mUnderlyingType)->IsAbstract())
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

    // map the object so it can be reused
    {
        const ObjectPtr& typedObject = *BitCast<const ObjectPtr*>(outObject);

        NFE_ASSERT(!context.IsObjectMapped(typedObject.Get()), "The object should not be mapped yet");
        const uint32 mappedObjectID = context.MapObject(typedObject);

        if (objectID != UINT32_MAX && mappedObjectID != objectID)
        {
            NFE_LOG_ERROR("Object ID in the data does not match the one generated at runtime");
            return false;
        }
    }

    // deserialize the object
    return targetType->Deserialize(pointedData, config, *pointedObjectValue, context);
}

bool SharedPtrType::SerializeBinary(const void* object, OutputStream* stream, SerializationContext& context) const
{
    const ObjectPtr& typedObject = *BitCast<const ObjectPtr*>(object);
    const Type* pointedDataType = GetPointedDataType(object);

    if (!TypeRegistry::GetInstance().SerializeTypeName(pointedDataType, stream, context))
    {
        return false;
    }

    if (pointedDataType)
    {
        if (!context.IsObjectMapped(typedObject.Get()))
        {
            // map object's content
            if (!pointedDataType->SerializeBinary(typedObject.Get(), nullptr, context))
            {
                return false;
            }
        }

        const uint32 objectIndex = context.MapObject(typedObject);
        if (!context.IsMapping())
        {
            if (!stream->WriteCompressedUint(objectIndex))
            {
                return false;
            }
        }

        // Note: the pointed object data is not serialized here, it's deferred and handled a by higher level serializer
    }

    return true;
}

bool SharedPtrType::DeserializeBinary(void* outObject, InputStream& stream, SerializationContext& context) const
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
        NFE_LOG_WARNING("Failed to deserialize SharedPtr of type '%s': type in data is not know. Probably the type was removed from code.", GetName().Str());
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
        NFE_LOG_WARNING("Failed to deserialize SharedPtr of type '%s': Type in data is '%s' and it's not compatible with underlying pointer type '%s'",
            GetName().Str(), serializedType->GetName().Str(), GetUnderlyingType()->GetName().Str());
        return false;
    }

    uint32 objIndex;
    if (!stream.ReadCompressedUint(objIndex))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return false;
    }

    // Note: the pointed object data is not deserialized here, it was already handled a by higher level serializer

    SharedPtr<IObject> unmappedObject;
    if (!context.UnmapObject(objIndex, unmappedObject))
    {
        NFE_LOG_ERROR("Failed to unmap object with index %u. Corrupted data?", objIndex);
        return false;
    }

    Assign(outObject, unmappedObject);

    NFE_ASSERT(GetPointedData(outObject) == unmappedObject.Get(), "Assignment failed");

    return true;
}

} // namespace RTTI
} // namespace NFE

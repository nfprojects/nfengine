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

SharedPtrType::SharedPtrType(const TypeInfo& info, const Type* underlyingType)
    : PointerType(info, underlyingType)
{
    NFE_ASSERT(mUnderlyingType->IsA(GetType<IObject>()), "Reflection: SharedPtr must point to IObject-based class");
}


bool SharedPtrType::Serialize(const void* object, IConfig& config, ConfigValue& outValue, SerializationContext& context) const
{
    NFE_ASSERT(object, "Trying to serialize nullptr");
    const void* pointedObject = GetPointedData(object);

    if (pointedObject)
    {
        mUnderlyingType->Serialize(pointedObject, config, outValue, context);
    }
    else // null pointer
    {
        outValue = ConfigValue(0);
    }

    return true;
}

bool SharedPtrType::Deserialize(void* outObject, const IConfig& config, const ConfigValue& value, const SerializationContext& context) const
{
    NFE_ASSERT(outObject, "Trying to deserialize to nullptr");

    NFE_UNUSED(outObject);
    NFE_UNUSED(config);
    NFE_UNUSED(value);
    NFE_UNUSED(context);

    NFE_ASSERT(false, "Not implemented!");

    return false;
}

bool SharedPtrType::SerializeBinary(const void* object, OutputStream* stream, SerializationContext& context) const
{
    const ObjectPtr& typedObject = *BitCast<const ObjectPtr*>(object);
    const Type* pointedDataType = GetPointedDataType(object);
    const StringView pointedDataTypeName = pointedDataType ? pointedDataType->GetName() : StringView();

    const uint32 typeNameStrIndex = context.MapString(pointedDataTypeName);
    if (!context.IsMapping())
    {
        if (!stream->WriteCompressedUint(typeNameStrIndex))
        {
            return false;
        }
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

bool SharedPtrType::DeserializeBinary(void* outObject, InputStream& stream, const SerializationContext& context) const
{
    // read pointed data type name
    uint32 strIndex;
    if (!stream.ReadCompressedUint(strIndex))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return false;
    }
    StringView serializedTypeName;
    if (!context.UnmapString(strIndex, serializedTypeName))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return false;
    }

    // handle nullptr
    if (serializedTypeName.Empty())
    {
        Reset(outObject);
        return true;
    }

    const Type* serializedType = TypeRegistry::GetInstance().GetExistingType(serializedTypeName);

    // serialized type not known
    if (!serializedType)
    {
        // TOOO skip object
        // TODO report unknown type
        NFE_LOG_WARNING("Failed to deserialize SharedPtr of type '%s': type in data '%.*s' is not know. Probably the type was removed from code.",
            GetName().Str(), serializedTypeName.Length(), serializedTypeName.Data());
        return false;
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

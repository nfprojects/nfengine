/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionUniquePtrType.hpp"
#include "../SerializationContext.hpp"
#include "../ReflectionTypeRegistry.hpp"
#include "../../Config/ConfigInterface.hpp"
#include "../../Utils/Stream/OutputStream.hpp"
#include "../../Utils/Stream/InputStream.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

bool UniquePtrType::SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const
{
    const Type* pointedDataType = GetPointedDataType(object);
    const StringView pointedDataTypeName = pointedDataType ? pointedDataType->GetName().ToView() : StringView();

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
        const void* pointedData = GetPointedData(object);
        NFE_ASSERT(pointedData, "Pointed data is expected to be valide if type is not null");

        if (!pointedDataType->SerializeBinary(pointedData, stream, context))
        {
            return false;
        }
    }

    return true;
}

bool UniquePtrType::DeserializeBinary(void* outObject, Common::InputStream& stream, const SerializationContext& context) const
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
        NFE_LOG_WARNING("Failed to deserialize UniquePtr of type '%s': type in data '%.*s' is not known. Probably the type was removed from code.",
            GetName().Str(), serializedTypeName.Length(), serializedTypeName.Data());
        return false;
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

    Reset(outObject, serializedType);
    void* pointedData = GetPointedData(outObject);
    if (!pointedData) // failed to allocate memory?
    {
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

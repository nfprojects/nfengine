/**
 * @file
 * @author Witek902
 * @brief  Definition of reflection system's StaticBufferType.
 */

#include "PCH.hpp"
#include "ReflectionStaticBufferType.hpp"
#include "../SerializationContext.hpp"
#include "../../Config/ConfigInterface.hpp"
#include "../../Utils/Stream/OutputStream.hpp"
#include "../../Utils/Stream/InputStream.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

StaticBufferType::StaticBufferType() = default;

StaticBufferType::~StaticBufferType() = default;

bool StaticBufferType::Serialize(const void* object, IConfig& config, ConfigValue& outValue, SerializationContext& context) const
{
    NFE_UNUSED(config);
    NFE_UNUSED(context);
    NFE_UNUSED(object);
    NFE_UNUSED(outValue);

    // TODO base64

    return false;
}

bool StaticBufferType::Deserialize(void* outObject, const IConfig& config, const ConfigValue& value, SerializationContext& context) const
{
    NFE_UNUSED(config);
    NFE_UNUSED(context);
    NFE_UNUSED(outObject);
    NFE_UNUSED(value);

    // TODO base64

    return false;
}

bool StaticBufferType::SerializeBinary(const void* object, OutputStream* stream, SerializationContext& context) const
{
    NFE_ASSERT(object);

    if (!context.IsMapping())
    {
        const StaticBuffer& typedObject = *static_cast<const StaticBuffer*>(object);

        const uint64 bufferSize = typedObject.Size();
        if (!stream->WriteCompressedUint(bufferSize))
        {
            return false;
        }

        if (typedObject.Size() > 0u)
        {
            if (!stream->Write(typedObject.Data(), typedObject.Size()))
            {
                return false;
            }
        }
    }

    return true;
}

bool StaticBufferType::DeserializeBinary(void* outObject, InputStream& stream, SerializationContext& context) const
{
    NFE_UNUSED(context);
    NFE_ASSERT(outObject);

    uint64 bufferSize = 0;
    if (!stream.ReadCompressedUint(bufferSize))
    {
        return false;
    }

    StaticBuffer& typedObject = *static_cast<StaticBuffer*>(outObject);
    if (!typedObject.Set(bufferSize))
    {
        NFE_LOG_ERROR("Failed to allocate buffer");
        return false;
    }

    if (bufferSize > 0u)
    {
        if (bufferSize > std::numeric_limits<size_t>::max())
        {
            NFE_LOG_ERROR("Buffer size is too big (%llu bytes)", bufferSize);
            return false;
        }

        const size_t bytesRead = stream.Read(typedObject.Data(), static_cast<size_t>(bufferSize));
        if (static_cast<size_t>(bufferSize) != bytesRead)
        {
            NFE_LOG_ERROR("Failed to read buffer: read %zu bytes, expected %llu bytes", bytesRead, bufferSize);
            return false;
        }
    }

    return true;
}

bool StaticBufferType::Compare(const void* objectA, const void* objectB) const
{
    NFE_ASSERT(objectA);
    NFE_ASSERT(objectB);

    const StaticBuffer& typedObjectA = *static_cast<const StaticBuffer*>(objectA);
    const StaticBuffer& typedObjectB = *static_cast<const StaticBuffer*>(objectB);
    return typedObjectA.ToView() == typedObjectB.ToView();
}

bool StaticBufferType::Clone(void* destObject, const void* sourceObject) const
{
    NFE_ASSERT(destObject);
    NFE_ASSERT(sourceObject);

    *static_cast<StaticBuffer*>(destObject) = *static_cast<const StaticBuffer*>(sourceObject);
    return true;
}

} // namespace RTTI
} // namespace NFE

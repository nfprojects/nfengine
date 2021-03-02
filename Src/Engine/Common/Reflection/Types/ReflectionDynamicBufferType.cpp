/**
 * @file
 * @author Witek902
 * @brief  Definition of reflection system's DynamicBufferType.
 */

#include "PCH.hpp"
#include "ReflectionDynamicBufferType.hpp"
#include "../SerializationContext.hpp"
#include "../../Config/ConfigInterface.hpp"
#include "../../Utils/Stream/OutputStream.hpp"
#include "../../Utils/Stream/InputStream.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

DynamicBufferType::DynamicBufferType() = default;

DynamicBufferType::~DynamicBufferType() = default;

bool DynamicBufferType::Deserialize(void* outObject, const IConfig& config, const ConfigValue& value, SerializationContext& context) const
{
    NFE_UNUSED(config);
    NFE_UNUSED(context);
    NFE_UNUSED(outObject);
    NFE_UNUSED(value);

    // TODO base64

    return false;
}

bool DynamicBufferType::DeserializeBinary(void* outObject, InputStream& stream, SerializationContext& context) const
{
    NFE_UNUSED(context);
    NFE_ASSERT(outObject);

    uint64 bufferSize = 0;
    if (!stream.ReadCompressedUint(bufferSize))
    {
        return false;
    }

    DynamicBuffer& typedObject = *static_cast<DynamicBuffer*>(outObject);
    if (!typedObject.Resize(bufferSize))
    {
        NFE_LOG_ERROR("Failed to allocate buffer");
        return false;
    }

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

    return true;
}

bool DynamicBufferType::Clone(void* destObject, const void* sourceObject) const
{
    NFE_ASSERT(destObject);
    NFE_ASSERT(sourceObject);

    *static_cast<DynamicBuffer*>(destObject) = *static_cast<const DynamicBuffer*>(sourceObject);
    return true;
}

} // namespace RTTI
} // namespace NFE

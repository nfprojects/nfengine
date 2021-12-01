/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's buffer
 */

#include "PCH.hpp"
#include "Buffer.hpp"
#include "Device.hpp"
#include "Internal/Debugger.hpp"


namespace NFE {
namespace Renderer {

Buffer::Buffer()
    : mBuffer(VK_NULL_HANDLE)
    , mBufferMemory(VK_NULL_HANDLE)
    , mBufferSize(0)
    , mMode(ResourceAccessMode::Invalid)
    , mVolatileBinding(UINT32_MAX)
    , mVolatileDataOffset(UINT32_MAX)
{
}

Buffer::~Buffer()
{
    if (mBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(gDevice->GetDevice(), mBuffer, nullptr);
    if (mBufferMemory != VK_NULL_HANDLE)
        vkFreeMemory(gDevice->GetDevice(), mBufferMemory, nullptr);
}

bool Buffer::Init(const BufferDesc& desc)
{
    VkResult result = VK_SUCCESS;

    // Temporary early leave until below types are implemented
    if (desc.mode == ResourceAccessMode::GPUOnly || desc.mode == ResourceAccessMode::Readback)
    {
        NFE_LOG_ERROR("Requested unsupported buffer mode");
        return false;
    }

    mMode = desc.mode;
    mBufferSize = static_cast<VkDeviceSize>(desc.size);

    VkBufferCreateInfo bufInfo;
    VK_ZERO_MEMORY(bufInfo);
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = mBufferSize;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkMemoryAllocateInfo memInfo;
    VK_ZERO_MEMORY(memInfo);
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    // Determine base buffer usage
    bufInfo.usage = TranslateBufferUsageToVkBufferUsage(desc.usage);

    result = vkCreateBuffer(gDevice->GetDevice(), &bufInfo, nullptr, &mBuffer);
    CHECK_VKRESULT(result, "Failed to create device buffer");

    if (desc.debugName)
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mBuffer), VK_OBJECT_TYPE_BUFFER, desc.debugName);

    VkMemoryRequirements deviceMemReqs;
    vkGetBufferMemoryRequirements(gDevice->GetDevice(), mBuffer, &deviceMemReqs);

    memInfo.allocationSize = deviceMemReqs.size;
    memInfo.memoryTypeIndex = gDevice->GetMemoryTypeIndex(deviceMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    result = vkAllocateMemory(gDevice->GetDevice(), &memInfo, nullptr, &mBufferMemory);
    CHECK_VKRESULT(result, "Failed to allocate memory for device buffer");

    if (desc.debugName)
    {
        Common::String memName(desc.debugName);
        memName += "-DeviceMemory";
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mBufferMemory), VK_OBJECT_TYPE_DEVICE_MEMORY, memName.Str());
    }

    result = vkBindBufferMemory(gDevice->GetDevice(), mBuffer, mBufferMemory, 0);
    CHECK_VKRESULT(result, "Failed to bind device buffer to its memory");

    std::string resourceAccessModeStr;
    switch (mMode)
    {
    case ResourceAccessMode::Immutable: resourceAccessModeStr = "Immutable"; break;
    case ResourceAccessMode::GPUOnly: resourceAccessModeStr = "GPUOnly"; break;
    case ResourceAccessMode::Upload: resourceAccessModeStr = "Upload"; break;
    case ResourceAccessMode::Volatile: resourceAccessModeStr = "Volatile"; break;
    case ResourceAccessMode::Readback: resourceAccessModeStr = "Readback"; break;
    default: resourceAccessModeStr = "Unknown";
    }

    NFE_LOG_INFO("%u-byte %s Buffer created successfully", desc.size, resourceAccessModeStr.c_str());
    return true;
}

void* Buffer::Map(size_t size, size_t offset)
{
    void* ptr = nullptr;
    VkResult result = VK_SUCCESS;

    result = vkMapMemory(gDevice->GetDevice(), mBufferMemory, offset, size ? size : VK_WHOLE_SIZE, 0, &ptr);
    if (result != VK_SUCCESS)
    {
        NFE_LOG_ERROR("Failed to map Buffer memory: %x (%s)", result, TranslateVkResultToString(result));
        return nullptr;
    }

    return ptr;
}

void Buffer::Unmap()
{
    vkUnmapMemory(gDevice->GetDevice(), mBufferMemory);
}

} // namespace Renderer
} // namespace NFE

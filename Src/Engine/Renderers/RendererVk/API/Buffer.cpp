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
    , mStructureSize(0)
    , mMode(ResourceAccessMode::Invalid)
    , mVolatileDataOffset(UINT32_MAX)
{
}

Buffer::~Buffer()
{
    if (mView != VK_NULL_HANDLE)
        vkDestroyBufferView(gDevice->GetDevice(), mView, nullptr);
    if (mBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(gDevice->GetDevice(), mBuffer, nullptr);
    if (mBufferMemory != VK_NULL_HANDLE)
        vkFreeMemory(gDevice->GetDevice(), mBufferMemory, nullptr);
}

bool Buffer::Init(const BufferDesc& desc)
{
    VkResult result = VK_SUCCESS;

    mMode = desc.mode;
    mUsage = desc.usage;
    mBufferSize = static_cast<VkDeviceSize>(desc.size);
    mStructureSize = static_cast<VkDeviceSize>(desc.structSize);

    // Volatile buffers are handled via Ring Buffer - no need for Buffer allocation
    if (desc.mode == ResourceAccessMode::Volatile)
        goto finish;

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
    bufInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    if (((bufInfo.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) &&
         (bufInfo.usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)) ||
        ((bufInfo.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) &&
         (bufInfo.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)))
    {
        NFE_LOG_ERROR("Usage combines texel and non-texel buffer flags - this is unsupported");
        NFE_LOG_ERROR("If this should be supported, scream at me @Lookey");
        return false;
    }

    result = vkCreateBuffer(gDevice->GetDevice(), &bufInfo, nullptr, &mBuffer);
    CHECK_VKRESULT(result, "Failed to create device buffer");

    if (desc.debugName)
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mBuffer), VK_OBJECT_TYPE_BUFFER, desc.debugName);

    VkMemoryRequirements deviceMemReqs;
    vkGetBufferMemoryRequirements(gDevice->GetDevice(), mBuffer, &deviceMemReqs);

    VkMemoryPropertyFlags mempropFlags;
    switch (desc.mode)
    {
    case ResourceAccessMode::GPUOnly:
    case ResourceAccessMode::Immutable:
        mempropFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case ResourceAccessMode::Readback:
    case ResourceAccessMode::Upload:
        mempropFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;
    default:
        NFE_ASSERT(0, "Invalid Resource Access Mode");
        return false;
    }

    memInfo.allocationSize = deviceMemReqs.size;
    memInfo.memoryTypeIndex = gDevice->GetMemoryTypeIndex(deviceMemReqs.memoryTypeBits, mempropFlags);
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


    // view
    VkBufferViewCreateInfo viewInfo;
    VK_ZERO_MEMORY(viewInfo);
    viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    viewInfo.buffer = mBuffer;
    viewInfo.format = VK_FORMAT_R32_UINT; // TODO
    viewInfo.offset = 0;
    viewInfo.range = VK_WHOLE_SIZE;
    result = vkCreateBufferView(gDevice->GetDevice(), &viewInfo, nullptr, &mView);
    CHECK_VKRESULT(result, "Failed to create View for whole buffer");

finish:
    NFE_LOG_INFO("%u-byte %s Buffer created successfully", desc.size, TranslateResourceAccessModeToString(mMode));
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

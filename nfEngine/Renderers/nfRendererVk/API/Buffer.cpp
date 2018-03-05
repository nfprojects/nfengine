/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's buffer
 */

#include "PCH.hpp"
#include "Buffer.hpp"
#include "Device.hpp"


namespace NFE {
namespace Renderer {

Buffer::Buffer()
    : mBuffer(VK_NULL_HANDLE)
    , mBufferMemory(VK_NULL_HANDLE)
    , mBufferSize(0)
    , mMode(BufferMode::Static)
{
}

Buffer::~Buffer()
{
    // TODO this needs to go away
    mDevicePtr->WaitForGPU();

    if (mBufferMemory != VK_NULL_HANDLE)
        vkFreeMemory(mDevicePtr->GetDevice(), mBufferMemory, nullptr);
    if (mBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(mDevicePtr->GetDevice(), mBuffer, nullptr);
}

bool Buffer::Init(Common::SharedPtr<Device> device, const BufferDesc& desc)
{
    mDevicePtr = device;

    VkResult result = VK_SUCCESS;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;

    // Temporary early leave until below types are implemented
    if (desc.mode == BufferMode::GPUOnly || desc.mode == BufferMode::Readback)
    {
        NFE_LOG_ERROR("Requested unsupported buffer mode");
        return false;
    }

    mMode = desc.mode;
    mBufferSize = static_cast<VkDeviceSize>(desc.size);

    if (desc.mode == BufferMode::Volatile && desc.type == BufferType::Constant)
        goto leave;

    if (desc.mode == BufferMode::Static && desc.initialData == nullptr)
    {
        NFE_LOG_ERROR("Cannot create a Static buffer without initial data provided");
        return false;
    }

    mBufferSize = static_cast<VkDeviceSize>(desc.size);

    VkBufferCreateInfo bufInfo;
    VK_ZERO_MEMORY(bufInfo);
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = mBufferSize;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkMemoryAllocateInfo memInfo;
    VK_ZERO_MEMORY(memInfo);
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    // staging buffer (only if there will be data to copy)
    if (desc.initialData)
    {
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        result = vkCreateBuffer(mDevicePtr->GetDevice(), &bufInfo, nullptr, &stagingBuffer);
        VK_RETURN_FALSE_IF_FAILED(result, "Failed to create staging buffer for data upload");

        VkMemoryRequirements stagingMemReqs;
        vkGetBufferMemoryRequirements(mDevicePtr->GetDevice(), stagingBuffer, &stagingMemReqs);

        memInfo.allocationSize = stagingMemReqs.size;
        memInfo.memoryTypeIndex = mDevicePtr->GetMemoryTypeIndex(stagingMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        result = vkAllocateMemory(mDevicePtr->GetDevice(), &memInfo, nullptr, &stagingBufferMemory);
        VK_RETURN_FALSE_IF_FAILED(result, "Failed to allocate memory for staging buffer");

        void* bufferData = nullptr;
        result = vkMapMemory(mDevicePtr->GetDevice(), stagingBufferMemory, 0, memInfo.allocationSize, 0, &bufferData);
        VK_RETURN_FALSE_IF_FAILED(result, "Failed to map staging buffer's memory");
        memcpy(bufferData, desc.initialData, desc.size);
        vkUnmapMemory(mDevicePtr->GetDevice(), stagingBufferMemory);
        result = vkBindBufferMemory(mDevicePtr->GetDevice(), stagingBuffer, stagingBufferMemory, 0);
        VK_RETURN_FALSE_IF_FAILED(result, "Failed to bind staging buffer to its memory");
    }


    // device buffer
    bufInfo.usage = TranslateBufferTypeToVkBufferUsage(desc.type) | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    result = vkCreateBuffer(mDevicePtr->GetDevice(), &bufInfo, nullptr, &mBuffer);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to create device buffer");

    VkMemoryRequirements deviceMemReqs;
    vkGetBufferMemoryRequirements(mDevicePtr->GetDevice(), mBuffer, &deviceMemReqs);

    memInfo.allocationSize = deviceMemReqs.size;
    memInfo.memoryTypeIndex = mDevicePtr->GetMemoryTypeIndex(deviceMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    result = vkAllocateMemory(mDevicePtr->GetDevice(), &memInfo, nullptr, &mBufferMemory);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to allocate memory for device buffer");

    result = vkBindBufferMemory(mDevicePtr->GetDevice(), mBuffer, mBufferMemory, 0);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to bind device buffer to its memory");


    // copy data from staging to device buffer (if there is anything to copy)
    if (desc.initialData)
    {
        // TODO right now copying is done on a general queue, but the devices support separate Transfer queue.
        //      Consider moving copy command buffers to transfer queue if device makes it possible.
        VkCommandBuffer copyCmdBuffer;
        VkCommandBufferAllocateInfo cmdInfo;
        VK_ZERO_MEMORY(cmdInfo);
        cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdInfo.commandPool = mDevicePtr->GetCommandPool(QueueType::General);
        cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdInfo.commandBufferCount = 1;
        result = vkAllocateCommandBuffers(mDevicePtr->GetDevice(), &cmdInfo, &copyCmdBuffer);
        VK_RETURN_FALSE_IF_FAILED(result, "Failed to allocate a command buffer");

        VkCommandBufferBeginInfo cmdBeginInfo;
        VK_ZERO_MEMORY(cmdBeginInfo);
        cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        result = vkBeginCommandBuffer(copyCmdBuffer, &cmdBeginInfo);
        VK_RETURN_FALSE_IF_FAILED(result, "Failed to begin command rendering for buffer copy operation");

        VkBufferCopy region;
        VK_ZERO_MEMORY(region);
        region.size = deviceMemReqs.size;
        vkCmdCopyBuffer(copyCmdBuffer, stagingBuffer, mBuffer, 1, &region);

        result = vkEndCommandBuffer(copyCmdBuffer);
        VK_RETURN_FALSE_IF_FAILED(result, "Failure during copy command buffer recording");

        VkSubmitInfo submitInfo;
        VK_ZERO_MEMORY(submitInfo);
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &copyCmdBuffer;
        // FIXME vkQueueSubmit(mDevicePtr->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

        mDevicePtr->WaitForGPU();

        vkFreeCommandBuffers(mDevicePtr->GetDevice(), mDevicePtr->GetCommandPool(QueueType::General), 1, &copyCmdBuffer);
        vkFreeMemory(mDevicePtr->GetDevice(), stagingBufferMemory, nullptr);
        vkDestroyBuffer(mDevicePtr->GetDevice(), stagingBuffer, nullptr);
    }

leave:
    char* bufferModeStr = nullptr;
    switch (mMode)
    {
    case BufferMode::Static: bufferModeStr = "Static"; break;
    case BufferMode::Dynamic: bufferModeStr = "Dynamic"; break;
    case BufferMode::Volatile: bufferModeStr = "Volatile"; break;
    case BufferMode::GPUOnly: bufferModeStr = "GPUOnly"; break;
    case BufferMode::Readback: bufferModeStr = "Readback"; break;
    default: bufferModeStr = "Unknown";
    }

    NFE_LOG_INFO("%u-byte %s Buffer created successfully", desc.size, bufferModeStr);
    return true;
}

} // namespace Renderer
} // namespace NFE

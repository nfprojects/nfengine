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
{
}

Buffer::~Buffer()
{
    if (mBufferMemory != VK_NULL_HANDLE)
        vkFreeMemory(gDevice->GetDevice(), mBufferMemory, nullptr);
    if (mBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(gDevice->GetDevice(), mBuffer, nullptr);
}

bool Buffer::Init(const BufferDesc& desc)
{
    // TODO access is unused!!

    VkResult result = VK_SUCCESS;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;

    VkBufferCreateInfo bufInfo;
    VK_ZERO_MEMORY(bufInfo);
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = static_cast<VkDeviceSize>(desc.size);

    VkMemoryAllocateInfo memInfo;
    VK_ZERO_MEMORY(memInfo);
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    // staging buffer (only if there will be data to copy)
    if (desc.initialData)
    {
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        result = vkCreateBuffer(gDevice->GetDevice(), &bufInfo, nullptr, &stagingBuffer);
        CHECK_VKRESULT(result, "Failed to create staging buffer for data upload");

        VkMemoryRequirements stagingMemReqs;
        vkGetBufferMemoryRequirements(gDevice->GetDevice(), stagingBuffer, &stagingMemReqs);

        memInfo.allocationSize = stagingMemReqs.size;
        memInfo.memoryTypeIndex = gDevice->GetMemoryTypeIndex(stagingMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        result = vkAllocateMemory(gDevice->GetDevice(), &memInfo, nullptr, &stagingBufferMemory);
        CHECK_VKRESULT(result, "Failed to allocate memory for staging buffer");

        void* bufferData = nullptr;
        result = vkMapMemory(gDevice->GetDevice(), stagingBufferMemory, 0, memInfo.allocationSize, 0, &bufferData);
        CHECK_VKRESULT(result, "Failed to map staging buffer's memory");
        memcpy(bufferData, desc.initialData, desc.size);
        vkUnmapMemory(gDevice->GetDevice(), stagingBufferMemory);
        result = vkBindBufferMemory(gDevice->GetDevice(), stagingBuffer, stagingBufferMemory, 0);
        CHECK_VKRESULT(result, "Failed to bind staging buffer to its memory");
    }


    // device buffer
    bufInfo.usage = TranslateBufferTypeToVkBufferUsage(desc.type) | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    result = vkCreateBuffer(gDevice->GetDevice(), &bufInfo, nullptr, &mBuffer);
    CHECK_VKRESULT(result, "Failed to create device buffer");

    VkMemoryRequirements deviceMemReqs;
    vkGetBufferMemoryRequirements(gDevice->GetDevice(), mBuffer, &deviceMemReqs);

    memInfo.allocationSize = deviceMemReqs.size;
    memInfo.memoryTypeIndex = gDevice->GetMemoryTypeIndex(deviceMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    result = vkAllocateMemory(gDevice->GetDevice(), &memInfo, nullptr, &mBufferMemory);
    CHECK_VKRESULT(result, "Failed to allocate memory for device buffer");

    result = vkBindBufferMemory(gDevice->GetDevice(), mBuffer, mBufferMemory, 0);
    CHECK_VKRESULT(result, "Failed to bind device buffer to its memory");


    // copy data from staging to device buffer (if there is anything to copy)
    if (desc.initialData)
    {
        // TODO right now copying is done on a general queue, but the devices support separate Transfer queue.
        //      Consider moving copy command buffers to transfer queue if device makes it possible.
        VkCommandBuffer copyCmdBuffer;
        VkCommandBufferAllocateInfo cmdInfo;
        VK_ZERO_MEMORY(cmdInfo);
        cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdInfo.commandPool = gDevice->GetCommandPool();
        cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdInfo.commandBufferCount = 1;
        result = vkAllocateCommandBuffers(gDevice->GetDevice(), &cmdInfo, &copyCmdBuffer);
        CHECK_VKRESULT(result, "Failed to allocate a command buffer");

        VkCommandBufferBeginInfo cmdBeginInfo;
        VK_ZERO_MEMORY(cmdBeginInfo);
        cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        result = vkBeginCommandBuffer(copyCmdBuffer, &cmdBeginInfo);
        CHECK_VKRESULT(result, "Failed to begin command rendering for buffer copy operation");

        VkBufferCopy region;
        VK_ZERO_MEMORY(region);
        region.size = deviceMemReqs.size;
        vkCmdCopyBuffer(copyCmdBuffer, stagingBuffer, mBuffer, 1, &region);

        result = vkEndCommandBuffer(copyCmdBuffer);
        CHECK_VKRESULT(result, "Failure during copy command buffer recording");

        VkSubmitInfo submitInfo;
        VK_ZERO_MEMORY(submitInfo);
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &copyCmdBuffer;
        vkQueueSubmit(gDevice->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

        gDevice->WaitForGPU();

        vkFreeCommandBuffers(gDevice->GetDevice(), gDevice->GetCommandPool(), 1, &copyCmdBuffer);
        vkFreeMemory(gDevice->GetDevice(), stagingBufferMemory, nullptr);
        vkDestroyBuffer(gDevice->GetDevice(), stagingBuffer, nullptr);
    }

    LOG_INFO("%u-byte Buffer created successfully", desc.size);
    return true;
}

} // namespace Renderer
} // namespace NFE
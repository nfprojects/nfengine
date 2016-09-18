/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Ring Buffer implementation for nfRendererVk
 */

#include "PCH.hpp"
#include "RingBuffer.hpp"
#include "Device.hpp"

namespace NFE {
namespace Renderer {

RingBuffer::RingBuffer()
    : mBuffer(VK_NULL_HANDLE)
    , mBufferMemory(VK_NULL_HANDLE)
    , mSize(0)
    , mHead(0)
    , mTail(0)
{
}

RingBuffer::~RingBuffer()
{
    vkUnmapMemory(gDevice->GetDevice(), mBufferMemory);

    if (mBufferMemory != VK_NULL_HANDLE)
        vkFreeMemory(gDevice->GetDevice(), mBufferMemory, nullptr);
    if (mBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(gDevice->GetDevice(), mBuffer, nullptr);
}

bool RingBuffer::Init(uint32 size)
{
    VkBufferCreateInfo bufInfo;
    VK_ZERO_MEMORY(bufInfo);
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = static_cast<VkDeviceSize>(size);
    bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    // device buffer
    VkResult result = vkCreateBuffer(gDevice->GetDevice(), &bufInfo, nullptr, &mBuffer);
    CHECK_VKRESULT(result, "Failed to create device buffer");

    VkMemoryRequirements deviceMemReqs;
    vkGetBufferMemoryRequirements(gDevice->GetDevice(), mBuffer, &deviceMemReqs);

    VkMemoryAllocateInfo memInfo;
    VK_ZERO_MEMORY(memInfo);
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memInfo.allocationSize = deviceMemReqs.size;
    memInfo.memoryTypeIndex = gDevice->GetMemoryTypeIndex(deviceMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    result = vkAllocateMemory(gDevice->GetDevice(), &memInfo, nullptr, &mBufferMemory);
    CHECK_VKRESULT(result, "Failed to allocate memory for device buffer");

    result = vkBindBufferMemory(gDevice->GetDevice(), mBuffer, mBufferMemory, 0);
    CHECK_VKRESULT(result, "Failed to bind device buffer to its memory");

    result = vkMapMemory(gDevice->GetDevice(), mBufferMemory, 0, deviceMemReqs.size, 0, reinterpret_cast<void**>(&mBufferHostMemory));
    CHECK_VKRESULT(result, "Failed to map memory to host");

    mSize = static_cast<uint32>(deviceMemReqs.size);

    return true;
}

void RingBuffer::Write(void* data, uint32 size)
{
    if (mUsed == mSize)
        return; // filled and cannot write

    memcpy(mBufferHostMemory + mTail, data, size);

    // update pointer

    mTail += size;
}

void RingBuffer::FinishFrame(uint64 index)
{

}

void RingBuffer::OnFrameCompleted(uint64 index)
{

}

} // namespace Renderer
} // namespace NFE

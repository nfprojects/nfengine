/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Ring Buffer implementation for nfRendererVk
 */

#include "PCH.hpp"
#include "RingBuffer.hpp"
#include "API/Device.hpp"

namespace NFE {
namespace Renderer {

RingBuffer::RingBuffer()
    : mDevicePtr()
    , mBuffer(VK_NULL_HANDLE)
    , mBufferMemory(VK_NULL_HANDLE)
    , mSize(0)
    , mHead(0)
    , mTail(0)
{
}

RingBuffer::~RingBuffer()
{
    Release();
}

bool RingBuffer::Init(Device* device, uint32 size)
{
    mDevicePtr = device;

    VkBufferCreateInfo bufInfo;
    VK_ZERO_MEMORY(bufInfo);
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = static_cast<VkDeviceSize>(size);
    bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    // device buffer
    VkResult result = vkCreateBuffer(mDevicePtr->GetDevice(), &bufInfo, nullptr, &mBuffer);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to create device buffer");

    VkMemoryRequirements deviceMemReqs;
    vkGetBufferMemoryRequirements(mDevicePtr->GetDevice(), mBuffer, &deviceMemReqs);

    VkMemoryAllocateInfo memInfo;
    VK_ZERO_MEMORY(memInfo);
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memInfo.allocationSize = deviceMemReqs.size;
    // TODO this memory type probably should be changed
    memInfo.memoryTypeIndex = mDevicePtr->GetMemoryTypeIndex(deviceMemReqs.memoryTypeBits,
                                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    result = vkAllocateMemory(mDevicePtr->GetDevice(), &memInfo, nullptr, &mBufferMemory);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to allocate memory for device buffer");

    result = vkBindBufferMemory(mDevicePtr->GetDevice(), mBuffer, mBufferMemory, 0);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to bind device buffer to its memory");

    result = vkMapMemory(mDevicePtr->GetDevice(), mBufferMemory, 0, deviceMemReqs.size, 0, reinterpret_cast<void**>(&mBufferHostMemory));
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to map memory to host");

    mSize = static_cast<uint32>(deviceMemReqs.size);

    return true;
}

void RingBuffer::Release()
{
    if (mBufferMemory != VK_NULL_HANDLE)
    {
        vkUnmapMemory(mDevicePtr->GetDevice(), mBufferMemory);
        vkFreeMemory(mDevicePtr->GetDevice(), mBufferMemory, nullptr);
    }

    if (mBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mDevicePtr->GetDevice(), mBuffer, nullptr);
    }

    mBufferMemory = VK_NULL_HANDLE;
    mBuffer = VK_NULL_HANDLE;
}

uint32 RingBuffer::Write(const void* data, uint32 size)
{
    uint32 dataHead = mTail;

    if (dataHead + size > mSize)
        dataHead = 0; // we met buffer's top capacity, go back to its beginning

    if (dataHead + size == mHead)
        return std::numeric_limits<uint32>::max(); // TODO: here we are filled and cannot write. Resize.

    memcpy(mBufferHostMemory + dataHead, data, size);

    // update pointer
    size = (size + 255) & ~255;
    mTail = dataHead + size;

    return dataHead;
}

void RingBuffer::FinishFrame()
{
    mFinishedFrames.PushBack(mTail);
}

void RingBuffer::PopOldestFrame()
{
    mHead = mFinishedFrames.Front();
    mFinishedFrames.PopFront();
}

} // namespace Renderer
} // namespace NFE

/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Ring Buffer implementation for nfRendererVk
 */

#include "PCH.hpp"
#include "RingBuffer.hpp"
#include "API/Device.hpp"
#include "Debugger.hpp"


namespace NFE {
namespace Renderer {

RingBuffer::RingBuffer(VkDevice device)
    : mDevice(device)
    , mBuffer(VK_NULL_HANDLE)
    , mBufferMemory(VK_NULL_HANDLE)
    , mSize(0)
    , mHead(0)
    , mTail(0)
{
}

RingBuffer::~RingBuffer()
{
    vkUnmapMemory(mDevice, mBufferMemory);

    if (mBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(mDevice, mBuffer, nullptr);
    if (mBufferMemory != VK_NULL_HANDLE)
        vkFreeMemory(mDevice, mBufferMemory, nullptr);
}

bool RingBuffer::Init(uint32 size)
{
    VkBufferCreateInfo bufInfo;
    VK_ZERO_MEMORY(bufInfo);
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = static_cast<VkDeviceSize>(size);
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    // device buffer
    VkResult result = vkCreateBuffer(mDevice, &bufInfo, nullptr, &mBuffer);
    CHECK_VKRESULT(result, "Failed to create device buffer");

    Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mBuffer), VK_OBJECT_TYPE_BUFFER, "RingBuffer");

    VkMemoryRequirements deviceMemReqs;
    vkGetBufferMemoryRequirements(mDevice, mBuffer, &deviceMemReqs);

    VkMemoryAllocateInfo memInfo;
    VK_ZERO_MEMORY(memInfo);
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memInfo.allocationSize = deviceMemReqs.size;
    // TODO this memory type probably should be changed
    memInfo.memoryTypeIndex = gDevice->GetMemoryTypeIndex(deviceMemReqs.memoryTypeBits,
                                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    result = vkAllocateMemory(mDevice, &memInfo, nullptr, &mBufferMemory);
    CHECK_VKRESULT(result, "Failed to allocate memory for device buffer");

    Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mBufferMemory), VK_OBJECT_TYPE_DEVICE_MEMORY, "RingBuffer-DeviceMemory");

    result = vkBindBufferMemory(mDevice, mBuffer, mBufferMemory, 0);
    CHECK_VKRESULT(result, "Failed to bind device buffer to its memory");

    result = vkMapMemory(mDevice, mBufferMemory, 0, deviceMemReqs.size, 0, reinterpret_cast<void**>(&mBufferHostMemory));
    CHECK_VKRESULT(result, "Failed to map memory to host");

    mSize = static_cast<uint32>(deviceMemReqs.size);

    return true;
}

uint32 RingBuffer::Write(const void* data, uint32 size)
{
    // TODO RingBuffer must accept size_t instead of uint32
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

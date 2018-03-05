/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Ring Buffer declarations for nfRendererVk
 */

#pragma once

#include "Defines.hpp"

#include "../../../nfCommon/Containers/Deque.hpp"

namespace NFE {
namespace Renderer {

class RingBuffer
{
    Device* mDevicePtr;

    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    char* mBufferHostMemory; // char type to allow pointer arithmetics with 1 byte precision
    uint32 mSize;
    uint32 mHead;
    uint32 mTail;

    // pair: frame count, frame's tail
    Common::Deque<uint32> mFinishedFrames;

public:
    RingBuffer();
    ~RingBuffer();

    bool Init(Device* device, uint32 size);
    void Release();

    uint32 Write(const void* data, uint32 size);
    void FinishFrame();
    void PopOldestFrame();

    NFE_INLINE uint32 GetTail()
    {
        return mTail;
    }

    NFE_INLINE VkBuffer GetVkBuffer()
    {
        return mBuffer;
    }

    NFE_INLINE uint32 GetSize()
    {
        return mSize;
    }

    NFE_INLINE uint32 GetCurrentFrameHead()
    {
        return mFinishedFrames.Back();
    }
};

} // namespace Renderer
} // namespace NFE

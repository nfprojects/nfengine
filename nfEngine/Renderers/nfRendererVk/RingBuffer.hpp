/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Ring Buffer declarations for nfRendererVk
 */

#pragma once

#include "Defines.hpp"

namespace NFE {
namespace Renderer {

class RingBuffer
{
    VkBuffer mBuffer;
    VkDeviceMemory mBufferMemory;
    char* mBufferHostMemory;
    uint32 mSize;
    uint32 mUsed;
    uint32 mHead;
    uint32 mTail;

    //std::queue<

public:
    RingBuffer();
    ~RingBuffer();

    bool Init(uint32 size);
    void Write(void* data, uint32 size);

    void FinishFrame(uint64 index);
    void OnFrameCompleted(uint64 index);
};

} // namespace Renderer
} // namespace NFE

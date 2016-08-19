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
    uint32 mSize;
    uint32 mHead;
    uint32 mTail;

public:
    RingBuffer();
    ~RingBuffer();

    bool Init(uint32 size);
    void Write(void* data, uint32 size);
};

} // namespace Renderer
} // namespace NFE

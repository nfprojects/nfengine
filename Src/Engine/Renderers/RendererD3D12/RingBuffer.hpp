/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of helper class RingBuffer
 */

#pragma once

#include "Common.hpp"

#include "../../Common/Containers/Deque.hpp"
#include "../../Common/System/RWLock.hpp"

namespace NFE {
namespace Renderer {

class FenceData;

class RingBuffer
{
public:
    static const size_t INVALID_OFFSET;

    RingBuffer();
    ~RingBuffer();

    bool Init(size_t size);

    // Allocate space and return offset from the beginning of the buffer
    size_t Allocate(size_t size, size_t alignment);

    // should be called after a frame has been submitted
    void FinishFrame(const FenceData* fenceData, uint64 fenceValue);

    // should be called after GPU finished rendering the frame
    void OnFenceValueCompleted(const FenceData* fenceData, uint64 fenceValue);

    NFE_INLINE void* GetCpuAddress() const
    {
        return mCpuAddress;
    }

    NFE_INLINE D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() const
    {
        return mGpuAddress;
    }

    NFE_INLINE ID3D12Resource* GetD3DResource() const
    {
        return mBuffer.Get();
    }

private:
    friend class CommandRecorder;

    void* mCpuAddress;
    D3D12_GPU_VIRTUAL_ADDRESS mGpuAddress;
    D3DPtr<ID3D12Resource> mBuffer;

    size_t mHead;
    size_t mTail;
    size_t mSize;
    size_t mUsed;

    struct PendingFence
    {
        const FenceData* fenceData;
        uint64 fenceValue;
        size_t bufferOffset;
    };

    // queue holding 'tail' pointers for each frame
    Common::Deque<PendingFence> mCompletedFrames;

    Common::RWLock mLock;
};

} // namespace Renderer
} // namespace NFE

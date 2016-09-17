/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of helper class RingBuffer
 */

#pragma once

#include "Common.hpp"
#include "Texture.hpp"
#include "nfCommon\System\RWLock.hpp"

#include <queue>


namespace NFE {
namespace Renderer {

class RingBuffer
{
public:
    typedef std::pair<uint64, size_t> FinishedFrameOffset;

private:
    friend class CommandRecorder;

    Common::RWLock mLock;

    void* mCpuAddress;
    D3D12_GPU_VIRTUAL_ADDRESS mGpuAddress;
    D3DPtr<ID3D12Resource> mBuffer;

    size_t mHead;
    size_t mTail;
    size_t mSize;
    size_t mUsed;

    // queue holding 'tail' pointers for each frame
    std::queue<FinishedFrameOffset> mCompletedFrames;

public:
    static const size_t INVALID_OFFSET;

    RingBuffer();
    ~RingBuffer();

    bool Init(size_t size);

    // Allocate space and return offset from the beginning of the buffer
    size_t Allocate(size_t size);

    // should be called after a frame has been submitted
    void FinishFrame(uint64 frameIndex);

    // should be called after GPU finished rendering the frame
    void OnFrameCompleted(uint64 frameIndex);

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
};

} // namespace Renderer
} // namespace NFE

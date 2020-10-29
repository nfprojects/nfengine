/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of helper class RingBuffer
 */

#include "PCH.hpp"
#include "RingBuffer.hpp"
#include "RendererD3D12.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/Utils/ScopedLock.hpp"

namespace NFE {
namespace Renderer {

const size_t RingBuffer::INVALID_OFFSET = static_cast<size_t>(-1);

RingBuffer::RingBuffer()
    : mCpuAddress(nullptr)
    , mHead(0)
    , mTail(0)
    , mSize(0)
    , mUsed(0)
{

}

RingBuffer::~RingBuffer()
{
    NFE_ASSERT(mCompletedFrames.Empty(), "Ring buffer is still in use");
}

bool RingBuffer::Init(size_t size)
{
    // buffer size is required to be 256-byte aligned
    size = (size + 255) & ~255;

    D3D12_HEAP_PROPERTIES heapProperties;
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = static_cast<UINT64>(size);
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr;
    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommittedResource(&heapProperties,
                                                                      D3D12_HEAP_FLAG_NONE,
                                                                      &resourceDesc,
                                                                      D3D12_RESOURCE_STATE_GENERIC_READ,
                                                                      nullptr,
                                                                      IID_PPV_ARGS(mBuffer.GetPtr())));
    if (FAILED(hr))
        return false;

    if (!SetDebugName(mBuffer.Get(), Common::StringView("RingBuffer")))
    {
        NFE_LOG_WARNING("Failed to set debug name");
    }

    D3D12_RANGE range;
    range.Begin = 0;
    range.End = 0;
    if (FAILED(D3D_CALL_CHECK(mBuffer->Map(0, &range, &mCpuAddress))))
        return false;

    mGpuAddress = mBuffer->GetGPUVirtualAddress();
    mHead = mTail = mUsed = 0;
    mSize = size;

    return true;
}

size_t RingBuffer::Allocate(size_t size, size_t alignment)
{
    NFE_ASSERT(Math::IsPowerOfTwo(alignment), "Invalid alignment");

    // TODO make lockless
    NFE_SCOPED_LOCK(mLock);

    // buffer is full
    if (mUsed == mSize)
    {
        // TODO allocate new buffer?
        NFE_FATAL("Ring buffer overflow");
        return INVALID_OFFSET;
    }

    const size_t alignedTail = (mTail + alignment - 1u) & ~(alignment - 1u);

    if (alignedTail >= mHead)
    {
        if (alignedTail + size <= mSize)
        {
            mUsed += size + (alignedTail - mTail);
            mTail = alignedTail + size;
            return alignedTail;
        }
        else if (size <= mHead)
        {
            // space after the tail is too small - allocate from the buffer beginning
            mUsed += (mSize - mTail) + size;
            mTail = size;
            return 0;
        }
    }
    else if (mTail + size <= mHead)
    {
        mUsed += size + (alignedTail - mTail);
        mTail = alignedTail + size;
        return alignedTail;
    }

    // TODO allocate new buffer?
    NFE_FATAL("Ring buffer overflow");
    return INVALID_OFFSET;
}

void RingBuffer::FinishFrame(const FenceData* fenceData, uint64 fenceValue)
{
    // TODO make lockless
    NFE_SCOPED_LOCK(mLock);

    const PendingFence pendingFence{ fenceData, fenceValue, mTail };

    mCompletedFrames.PushBack(pendingFence);
}

void RingBuffer::OnFenceValueCompleted(const FenceData* fenceData, uint64 fenceValue)
{
    // TODO command queue - handle multiple queues
    NFE_UNUSED(fenceData);

    // TODO make lockless
    NFE_SCOPED_LOCK(mLock);

    while (!mCompletedFrames.Empty() && fenceValue >= mCompletedFrames.Front().fenceValue)
    {
        size_t oldestFrameTail = mCompletedFrames.Front().bufferOffset;
        size_t newUsed = 0;

        if (mUsed > 0)
        {
            if (oldestFrameTail >= mHead)
            {
                newUsed = mUsed - oldestFrameTail + mHead;
            }
            else
            {
                newUsed = mUsed - oldestFrameTail - mSize + mHead;
            }
        }

        NFE_ASSERT(newUsed <= mSize, "Ring buffer overrun");

        mUsed = newUsed;
        mHead = oldestFrameTail;
        mCompletedFrames.PopFront();
    }
}

} // namespace Renderer
} // namespace NFE

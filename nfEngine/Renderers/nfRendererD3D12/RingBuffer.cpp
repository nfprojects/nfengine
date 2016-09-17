/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of helper class RingBuffer
 */

#include "PCH.hpp"
#include "RingBuffer.hpp"
#include "RendererD3D12.hpp"
#include "nfCommon/Logger/Logger.hpp"


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
    NFE_ASSERT(mCompletedFrames.empty(), "Ring buffer is still in use");
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

size_t RingBuffer::Allocate(size_t size)
{
    // buffer is full
    if (mUsed == mSize)
        return INVALID_OFFSET;

    if (mTail >= mHead)
    {
        if (mTail + size <= mSize)
        {
            size_t offset = mTail;
            mTail += size;
            mUsed += size;
            return offset;
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
        auto Offset = mTail;
        mTail += size;
        mUsed += size;
        return Offset;
    }

    return INVALID_OFFSET;
}

void RingBuffer::FinishFrame(uint64 frameIndex)
{
    mCompletedFrames.push(std::make_pair(frameIndex, mTail));
}

void RingBuffer::OnFrameCompleted(uint64 frameIndex)
{
    NFE_ASSERT(!mCompletedFrames.empty(), "");
    NFE_ASSERT(mCompletedFrames.front().first == frameIndex, "");

    size_t oldestFrameTail = mCompletedFrames.front().second;
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
    mCompletedFrames.pop();
}

} // namespace Renderer
} // namespace NFE

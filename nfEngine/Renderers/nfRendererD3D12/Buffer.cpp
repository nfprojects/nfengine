/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's buffer
 */

#include "PCH.hpp"
#include "Buffer.hpp"
#include "RendererD3D12.hpp"
#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {

Buffer::Buffer()
    : mSize(0)
    , mData(nullptr)
{
}

bool Buffer::Init(const BufferDesc& desc)
{
    if (desc.access == BufferAccess::GPU_ReadWrite || desc.access == BufferAccess::CPU_Read)
    {
        LOG_ERROR("This access mode is not supported yet.");
        return false;
    }

    // buffer size is required to be 256-byte aligned
    mSize = (desc.size + 255) & ~255;

    D3D12_HEAP_PROPERTIES heapProperties;
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = mSize;
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
                                                                      IID_PPV_ARGS(&mResource)));
    if (FAILED(hr))
        return false;

    D3D12_RANGE range;
    range.Begin = 0;
    range.End = 0;
    if (FAILED(D3D_CALL_CHECK(mResource->Map(0, &range, &mData))))
        return false;

    // write initial data
    if (desc.initialData)
        memcpy(mData, desc.initialData, desc.size);

    if (desc.access == BufferAccess::GPU_ReadOnly)
    {
        mResource->Unmap(0, nullptr);
        mData = nullptr;

        if (!desc.initialData)
            LOG_WARNING("Initial data for GPU read-only buffer was not provided.");
    }

    mType = desc.type;
    mAccess = desc.access;
    return true;
}

} // namespace Renderer
} // namespace NFE

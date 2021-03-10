/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's buffer
 */

#include "PCH.hpp"
#include "Buffer.hpp"
#include "RendererD3D12.hpp"


namespace NFE {
namespace Renderer {

Buffer::Buffer()
    : Resource(D3D12_RESOURCE_STATE_COMMON)
    , mSize(0)
    , mStructureSize(0)
{
}

Buffer::~Buffer()
{
}

bool Buffer::Init(const BufferDesc& desc)
{
    if (desc.size == 0)
    {
        NFE_LOG_ERROR("Cannot create zero-sized buffer");
        return false;
    }

    if (desc.structSize > 0u)
    {
        NFE_ASSERT(desc.size % desc.structSize == 0, "Invalid struct size");
    }

    // buffer size is required to be 256-byte aligned
    mSize = static_cast<uint32>(desc.size);
    mStructureSize = desc.structSize;
    mMode = desc.mode;

    if (desc.mode == ResourceAccessMode::Volatile)
    {
        // volatile buffers are handled via ring buffer
        return true;
    }

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = GetRealSize();
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;

    if (desc.mode == ResourceAccessMode::GPUOnly || desc.mode == ResourceAccessMode::Immutable)
    {
        initialState = D3D12_RESOURCE_STATE_COMMON;
        allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
    }
    else if (desc.mode == ResourceAccessMode::Upload)
    {
        initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
        allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
    }
    else if (desc.mode == ResourceAccessMode::Readback)
    {
        initialState = D3D12_RESOURCE_STATE_COPY_DEST;
        allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
    }
    else
    {
        NFE_FATAL("Invalid resource access mode");
        return false;
    }

    if ((desc.usage & BufferUsageFlag::ReadonlyStruct) != BufferUsageFlag::ReadonlyStruct)
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }

    if ((desc.usage & BufferUsageFlag::WritableStruct) == BufferUsageFlag::WritableStruct)
    {
        NFE_ASSERT(desc.mode == ResourceAccessMode::GPUOnly, "Invalid access mode for writable buffer");

        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    mState = initialState;

    HRESULT hr = D3D_CALL_CHECK(gDevice->GetAllocator()->CreateResource(
        &allocationDesc,
        &resourceDesc,
        initialState,
        nullptr,
        mAllocation.GetPtr(),
        IID_PPV_ARGS(mResource.GetPtr())));

    if (FAILED(hr))
    {
        return false;
    }

    if (desc.debugName && !SetDebugName(mResource.Get(), Common::StringView(desc.debugName)))
    {
        NFE_LOG_WARNING("Failed to set debug name of a buffer");
    }

    return true;
}

void* Buffer::Map(size_t size, size_t offset)
{
    if (!mResource)
    {
        NFE_FATAL("Invalid resource");
        return nullptr;
    }

    if (size == 0)
    {
        // mapping whole resource
        size = mSize;
    }

    NFE_ASSERT(size <= mSize, "Buffer::Map is out of buffer range");
    NFE_ASSERT(offset < mSize, "Buffer::Map is out of buffer range");
    NFE_ASSERT(size + offset <= mSize, "Buffer::Map is out of buffer range");
    NFE_ASSERT(mMode == ResourceAccessMode::Upload || mMode == ResourceAccessMode::Readback, "Can only map upload or readback buffer");

    D3D12_RANGE range;
    range.Begin = size;
    range.End = size + offset;

    void* mappedData = nullptr;
    HRESULT hr = D3D_CALL_CHECK(mResource->Map(0, &range, &mappedData));

    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to map a buffer");
        return nullptr;
    }
    
    return mappedData;
}

void Buffer::Unmap()
{
    mResource->Unmap(0, NULL);
}

} // namespace Renderer
} // namespace NFE

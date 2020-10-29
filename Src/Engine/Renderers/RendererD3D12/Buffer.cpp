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
{
}

Buffer::~Buffer()
{
}

bool Buffer::UploadData(const BufferDesc& desc)
{
    NFE_UNUSED(desc);

    /*

    // Create temporary upload buffer on upload heap

    D3D12_HEAP_PROPERTIES heapProperties;
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    HRESULT hr;
    D3DPtr<ID3D12Resource> uploadBuffer;
    UINT64 requiredSize = 0;
    D3D12_RESOURCE_DESC d3dResDesc = mResource->GetDesc();
    gDevice->GetDevice()->GetCopyableFootprints(&d3dResDesc, 0, 1, 0,
                                                nullptr, nullptr, nullptr, &requiredSize);

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = requiredSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // TODO get rid of that, use existing ring buffer for resource uploads
    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommittedResource(&heapProperties,
                                                                      D3D12_HEAP_FLAG_NONE,
                                                                      &resourceDesc,
                                                                      D3D12_RESOURCE_STATE_GENERIC_READ,
                                                                      nullptr,
                                                                      IID_PPV_ARGS(uploadBuffer.GetPtr())));
    if (FAILED(hr))
    {
        return false;
    }

    if (desc.debugName)
    {
        const Common::String debugName = desc.debugName + Common::String("_UPLOAD");
        if (!SetDebugName(uploadBuffer.Get(), debugName))
        {
            NFE_LOG_WARNING("Failed to set debug name");
        }
    }

    if (desc.initialData)
    {
        // Create temporary command allocator and command list
        // TODO this is extremly inefficient

        D3DPtr<ID3D12CommandAllocator> commandAllocator;
        hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY,
                                                                         IID_PPV_ARGS(commandAllocator.GetPtr())));
        if (FAILED(hr))
            return false;

        SetDebugName(commandAllocator.Get(), "Buffer upload command allocator");

        if (FAILED(D3D_CALL_CHECK(commandAllocator->Reset())))
            return false;

        D3DPtr<ID3D12GraphicsCommandList> commandList;
        hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY,
                                                                    commandAllocator.Get(), nullptr,
                                                                    IID_PPV_ARGS(commandList.GetPtr())));
        if (FAILED(hr))
            return false;

        SetDebugName(commandList.Get(), "Buffer upload commandlist");

        if (FAILED(D3D_CALL_CHECK(commandList->Close())))
            return false;

        if (FAILED(D3D_CALL_CHECK(commandList->Reset(commandAllocator.Get(), nullptr))))
            return false;

        // Copy data to upload buffer

        char* mappedData;
        hr = D3D_CALL_CHECK(uploadBuffer->Map(0, NULL, reinterpret_cast<void**>(&mappedData)));
        if (FAILED(hr))
        {
            NFE_LOG_ERROR("Failed to map upload buffer");
            return false;
        }

        memcpy(mappedData, desc.initialData, desc.size);
        uploadBuffer->Unmap(0, NULL);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = mResource.Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        // resource must be in COMMON state before first use on copy queue
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        commandList->ResourceBarrier(1, &barrier);

        commandList->CopyResource(mResource.Get(), uploadBuffer.Get());

        // resource must be in COMMON state after use on copy queue
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
        commandList->ResourceBarrier(1, &barrier);

        // close the command list and send it to the command queue
        if (FAILED(D3D_CALL_CHECK(commandList->Close())))
        {
            return false;
        }

        ID3D12CommandList* commandLists[] = { commandList.Get() };
        gDevice->GetResourceUploadQueue()->ExecuteCommandLists(1, commandLists);

        // TODO get rid of wait, return the fence instead
        // then the command list and allocator must come from command list manager
        gDevice->mResourceUploadQueueFence.Signal(gDevice->mResourceUploadQueue.Get())->Wait();
    }
    */

    return true;
}

bool Buffer::Init(const BufferDesc& desc)
{
    if (desc.size == 0)
    {
        NFE_LOG_ERROR("Cannot create zero-sized buffer");
        return false;
    }

    // buffer size is required to be 256-byte aligned
    mSize = static_cast<uint32>(desc.size);
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

    if (desc.mode == ResourceAccessMode::GPUOnly || desc.mode == ResourceAccessMode::Static)
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
        NFE_LOG_WARNING("Failed to set debug name");
    }

    // write initial data if provided
    if (desc.initialData)
    {
        if (desc.mode == ResourceAccessMode::GPUOnly || desc.mode == ResourceAccessMode::Static)
        {
            if (!UploadData(desc))
            {
                return false;
            }
        }
        else if (desc.mode == ResourceAccessMode::Upload)
        {
            void* mappedData = nullptr;
            hr = D3D_CALL_CHECK(mResource->Map(0, NULL, &mappedData));

            if (SUCCEEDED(hr))
            {
                memcpy(mappedData, desc.initialData, desc.size);
                mResource->Unmap(0, NULL);
            }
            else
            {
                NFE_LOG_ERROR("Failed to map upload buffer");
                return false;
            }
        }
        else if (desc.mode == ResourceAccessMode::Readback)
        {
            NFE_LOG_ERROR("Readback buffers can't have initial data specified");
            return false;
        }
    }
    else if (desc.mode == ResourceAccessMode::Static)
    {
        NFE_LOG_ERROR("Initial data for static buffer was not provided.");
        return false;
    }

    return true;
}

} // namespace Renderer
} // namespace NFE

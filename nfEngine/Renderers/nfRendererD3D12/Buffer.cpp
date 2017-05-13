/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's buffer
 */

#include "PCH.hpp"
#include "Buffer.hpp"
#include "RendererD3D12.hpp"
#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

Buffer::Buffer()
    : mSize(0)
{
}

Buffer::~Buffer()
{
    gDevice->WaitForGPU();
}

bool Buffer::UploadData(const BufferDesc& desc)
{
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

    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommittedResource(&heapProperties,
                                                                      D3D12_HEAP_FLAG_NONE,
                                                                      &resourceDesc,
                                                                      D3D12_RESOURCE_STATE_GENERIC_READ,
                                                                      nullptr,
                                                                      IID_PPV_ARGS(&uploadBuffer)));

    if (desc.debugName && !SetDebugName(uploadBuffer.Get(), desc.debugName + std::string("_UPLOAD")))
    {
        LOG_WARNING("Failed to set debug name");
    }

    if (FAILED(hr))
        return false;


    if (desc.initialData)
    {
        // Create temporary command allocator and command list
        // TODO this is extremly inefficient

        D3DPtr<ID3D12CommandAllocator> commandAllocator;
        hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                                         IID_PPV_ARGS(&commandAllocator)));
        if (FAILED(hr))
            return false;

        if (FAILED(D3D_CALL_CHECK(commandAllocator->Reset())))
            return false;

        D3DPtr<ID3D12GraphicsCommandList> commandList;
        hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                                    commandAllocator.Get(), nullptr,
                                                                    IID_PPV_ARGS(&commandList)));
        if (FAILED(hr))
            return false;

        if (FAILED(D3D_CALL_CHECK(commandList->Close())))
            return false;

        if (FAILED(D3D_CALL_CHECK(commandList->Reset(commandAllocator.Get(), nullptr))))
            return false;

        // Copy data to upload buffer

        char* mappedData;
        hr = D3D_CALL_CHECK(uploadBuffer->Map(0, NULL, reinterpret_cast<void**>(&mappedData)));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to map upload buffer");
            return false;
        }

        memcpy(mappedData, desc.initialData, desc.size);
        uploadBuffer->Unmap(0, NULL);

        commandList->CopyResource(mResource.Get(), uploadBuffer.Get());

        // Enqueue resource barrier
        D3D12_RESOURCE_BARRIER resBarrier;
        resBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        resBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        resBarrier.Transition.pResource = mResource.Get();
        resBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        resBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        resBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
        commandList->ResourceBarrier(1, &resBarrier);


        // close the command list and send it to the command queue
        if (FAILED(D3D_CALL_CHECK(commandList->Close())))
            return false;
        ID3D12CommandList* commandLists[] = { commandList.Get() };
        gDevice->GetCommandQueue()->ExecuteCommandLists(1, commandLists);


        if (!gDevice->WaitForGPU())
            return false;
    }

    return true;
}

bool Buffer::Init(const BufferDesc& desc)
{
    if (desc.mode == BufferMode::GPUOnly || desc.mode == BufferMode::Readback)
    {
        LOG_ERROR("This access mode is not supported yet.");
        return false;
    }

    if (desc.size == 0)
    {
        LOG_ERROR("Cannot create zero-sized buffer");
        return false;
    }

    // buffer size is required to be 256-byte aligned
    mSize = static_cast<uint32>(desc.size);
    mType = desc.type;
    mMode = desc.mode;

    if (desc.mode == BufferMode::Volatile)
    {
        // volatile buffers are handled via ring buffer
        return true;
    }

    D3D12_HEAP_PROPERTIES heapProperties;
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

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

    D3D12_RESOURCE_STATES initialState = desc.initialData ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_GENERIC_READ;

    HRESULT hr;
    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommittedResource(&heapProperties,
                                                                      D3D12_HEAP_FLAG_NONE,
                                                                      &resourceDesc,
                                                                      initialState,
                                                                      nullptr,
                                                                      IID_PPV_ARGS(&mResource)));
    if (FAILED(hr))
        return false;

    if (desc.debugName && !SetDebugName(mResource.Get(), desc.debugName))
    {
        LOG_WARNING("Failed to set debug name");
    }

    // write initial data if provided
    if (desc.initialData)
    {
        if (!UploadData(desc))
            return false;
    }
    else if (desc.mode == BufferMode::Static)
    {
        LOG_ERROR("Initial data for static buffer was not provided.");
        return false;
    }

    return true;
}

} // namespace Renderer
} // namespace NFE

/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's texture
 */

#include "PCH.hpp"
#include "Texture.hpp"
#include "RendererD3D12.hpp"
#include "Translations.hpp"
#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {

Texture::Texture()
    : mBuffersNum(0)
    , mClass(Class::Regular)
    , mCurrentBuffer(0)
    , mResourceState(D3D12_RESOURCE_STATE_COMMON)
{
}

Texture::~Texture()
{
    gDevice->WaitForGPU();
}

bool Texture::UploadData(const TextureDesc& desc)
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
    D3D12_RESOURCE_DESC d3dResDesc = mBuffers[0]->GetDesc();

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts[D3D12_REQ_MIP_LEVELS];
    UINT64 rowSizesInBytes[D3D12_REQ_MIP_LEVELS];
    UINT numRows[D3D12_REQ_MIP_LEVELS];
    gDevice->GetDevice()->GetCopyableFootprints(&d3dResDesc, 0, desc.mipmaps, 0,
                                                layouts, numRows, rowSizesInBytes, &requiredSize);

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
    if (FAILED(hr))
        return false;


    if (desc.dataDesc)
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
                                                                    commandAllocator.get(), nullptr,
                                                                    IID_PPV_ARGS(&commandList)));
        if (FAILED(hr))
            return false;

        if (FAILED(D3D_CALL_CHECK(commandList->Close())))
            return false;

        if (FAILED(D3D_CALL_CHECK(commandList->Reset(commandAllocator.get(), nullptr))))
            return false;

        // Copy data to upload buffer

        char* data;
        hr = uploadBuffer->Map(0, NULL, reinterpret_cast<void**>(&data));
        if (FAILED(hr))
        {
            return 0;
        }

        for (int i = 0; i < desc.mipmaps; ++i)
        {
            D3D12_MEMCPY_DEST memcpyDest;
            memcpyDest.pData = data + layouts[i].Offset;
            memcpyDest.RowPitch = layouts[i].Footprint.RowPitch;
            memcpyDest.SlicePitch = layouts[i].Footprint.RowPitch * numRows[i];

            D3D12_SUBRESOURCE_DATA memcpySrc;
            memcpySrc.pData = desc.dataDesc[i].data;
            memcpySrc.RowPitch = desc.dataDesc[i].lineSize;
            memcpySrc.SlicePitch = desc.dataDesc[i].sliceSize;

            for (UINT z = 0; z < layouts[i].Footprint.Depth; ++z)
            {
                char* destSlice = reinterpret_cast<char*>(memcpyDest.pData) + memcpyDest.SlicePitch * z;
                const char* srcSlice = reinterpret_cast<const char*>(memcpySrc.pData) + memcpySrc.SlicePitch * z;

                for (UINT y = 0; y < numRows[i]; ++y)
                    memcpy(destSlice + memcpyDest.RowPitch * y,
                           srcSlice + memcpySrc.RowPitch * y,
                           rowSizesInBytes[i]);
            }
        }
        uploadBuffer->Unmap(0, NULL);

        // Copy data from upload buffer to the texture
        for (int i = 0; i < desc.mipmaps; ++i)
        {
            D3D12_TEXTURE_COPY_LOCATION src;
            src.pResource = uploadBuffer.get();
            src.PlacedFootprint = layouts[i];
            src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

            D3D12_TEXTURE_COPY_LOCATION dest;
            dest.pResource = mBuffers[0].get();
            dest.SubresourceIndex = i;
            dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

            commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
        }

        // Enqueue resource barrier
        D3D12_RESOURCE_BARRIER resBarrier;
        resBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        resBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        resBarrier.Transition.pResource = mBuffers[0].get();
        resBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        resBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        resBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        commandList->ResourceBarrier(1, &resBarrier);


        // close the command list and send it to the command queue
        if (FAILED(D3D_CALL_CHECK(commandList->Close())))
            return false;
        ID3D12CommandList* commandLists[] = { commandList.get() };
        gDevice->GetCommandQueue()->ExecuteCommandLists(1, commandLists);


        if (!gDevice->WaitForGPU())
            return false;
    }

    return true;
}

bool Texture::Init(const TextureDesc& desc)
{
    if (desc.access == BufferAccess::GPU_ReadWrite ||
        desc.access == BufferAccess::CPU_Read ||
        desc.access == BufferAccess::CPU_Write)
    {
        LOG_ERROR("This access mode is not supported yet");
        return false;
    }

    if (desc.samplesNum > 1)
    {
        LOG_ERROR("Multisampled textures are not supported yet");
        return false;
    }

    // Create texture resource on the default heap

    D3D12_HEAP_PROPERTIES heapProperties;
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Alignment = 0;
    resourceDesc.MipLevels = static_cast<UINT16>(desc.mipmaps);
    resourceDesc.Format = TranslateElementFormat(desc.format, desc.texelSize);
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    switch (desc.type)
    {
    case TextureType::Texture1D:
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        resourceDesc.Width = desc.width;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = static_cast<UINT16>(desc.layers);
        break;
    case TextureType::Texture2D:
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDesc.Width = desc.width;
        resourceDesc.Height = desc.height;
        resourceDesc.DepthOrArraySize = static_cast<UINT16>(desc.layers);
        break;
    case TextureType::TextureCube:
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDesc.Width = desc.width;
        resourceDesc.Height = desc.height;
        resourceDesc.DepthOrArraySize = 6 * static_cast<UINT16>(desc.layers);
        break;
    case TextureType::Texture3D:
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        resourceDesc.Width = desc.width;
        resourceDesc.Height = desc.height;
        resourceDesc.DepthOrArraySize = static_cast<UINT16>(desc.depth);
        break;
    }

    HRESULT hr;
    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommittedResource(&heapProperties,
                                                                      D3D12_HEAP_FLAG_NONE,
                                                                      &resourceDesc,
                                                                      D3D12_RESOURCE_STATE_GENERIC_READ,
                                                                      nullptr,
                                                                      IID_PPV_ARGS(&mBuffers[0])));
    if (FAILED(hr))
        return false;

    if (desc.access == BufferAccess::GPU_ReadOnly)
    {
        if (desc.dataDesc)
        {
            if (!UploadData(desc))
                return false;
        }
        else
            LOG_WARNING("No initial data for read-only texture provided");
    }


    mSrvFormat = resourceDesc.Format;
    mResourceState = D3D12_RESOURCE_STATE_COMMON;
    mBuffersNum = 1;
    mCurrentBuffer = 0;
    mType = desc.type;
    mMipmapsNum = desc.mipmaps;
    mLayersNum = desc.layers;
    return true;
}

} // namespace Renderer
} // namespace NFE
/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's texture
 */

#include "PCH.hpp"
#include "Texture.hpp"
#include "RendererD3D12.hpp"
#include "Translations.hpp"
#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

Texture::Texture()
    : mBuffersNum(1)
    , mCurrentBuffer(0)
    , mTargetState(D3D12_RESOURCE_STATE_COMMON)
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
                                                                      IID_PPV_ARGS(uploadBuffer.GetPtr())));
    if (FAILED(hr))
        return false;


    if (desc.dataDesc)
    {
        // Create temporary command allocator and command list
        // TODO this is extremly inefficient

        D3DPtr<ID3D12CommandAllocator> commandAllocator;
        hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                                         IID_PPV_ARGS(commandAllocator.GetPtr())));
        if (FAILED(hr))
            return false;

        if (FAILED(D3D_CALL_CHECK(commandAllocator->Reset())))
            return false;

        D3DPtr<ID3D12GraphicsCommandList> commandList;
        hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                                    commandAllocator.Get(), nullptr,
                                                                    IID_PPV_ARGS(commandList.GetPtr())));
        if (FAILED(hr))
            return false;

        if (FAILED(D3D_CALL_CHECK(commandList->Close())))
            return false;

        if (FAILED(D3D_CALL_CHECK(commandList->Reset(commandAllocator.Get(), nullptr))))
            return false;

        // Copy data to upload buffer

        char* data;
        hr = uploadBuffer->Map(0, NULL, reinterpret_cast<void**>(&data));
        if (FAILED(hr))
        {
            return 0;
        }

        for (uint32 i = 0; i < desc.mipmaps; ++i)
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
                    memcpy(destSlice + memcpyDest.RowPitch * y, srcSlice + memcpySrc.RowPitch * y,
                           static_cast<size_t>(rowSizesInBytes[i]));
            }
        }
        uploadBuffer->Unmap(0, NULL);

        // Copy data from upload buffer to the texture
        for (uint32 i = 0; i < desc.mipmaps; ++i)
        {
            D3D12_TEXTURE_COPY_LOCATION src;
            src.pResource = uploadBuffer.Get();
            src.PlacedFootprint = layouts[i];
            src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

            D3D12_TEXTURE_COPY_LOCATION dest;
            dest.pResource = mBuffers[0].Get();
            dest.SubresourceIndex = i;
            dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

            commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
        }

        // Enqueue resource barrier
        D3D12_RESOURCE_BARRIER resBarrier;
        resBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        resBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        resBarrier.Transition.pResource = mBuffers[0].Get();
        resBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        resBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        resBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
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

bool Texture::Init(const TextureDesc& desc)
{
    HRESULT hr;

    if (desc.mode == BufferMode::Dynamic || desc.mode == BufferMode::Volatile)
    {
        NFE_LOG_ERROR("Selected buffer mode is not supported yet");
        return false;
    }

    if (desc.samplesNum > 1)
    {
        NFE_LOG_ERROR("Multisampled textures are not supported yet");
        return false;
    }

    if (desc.width < 1 || desc.width >= std::numeric_limits<uint16>::max())
    {
        NFE_LOG_ERROR("Invalid texture width");
        return false;
    }

    if ((desc.type != TextureType::Texture1D) &&
        (desc.height < 1 || desc.height >= std::numeric_limits<uint16>::max()))
    {
        NFE_LOG_ERROR("Invalid texture height");
        return false;
    }

    if ((desc.type == TextureType::TextureCube) &&
        (desc.depth < 1 || desc.depth >= std::numeric_limits<uint16>::max()))
    {
        NFE_LOG_ERROR("Invalid texture depth");
        return false;
    }

    if (desc.layers < 1 || desc.layers >= std::numeric_limits<uint16>::max())
    {
        NFE_LOG_ERROR("Invalid number of layers");
        return false;
    }

    if (desc.mipmaps < 1 || desc.mipmaps >= D3D12_REQ_MIP_LEVELS)
    {
        NFE_LOG_ERROR("Invalid number of mipmaps");
        return false;
    }


    // Create texture resource on the default heap

    D3D12_HEAP_PROPERTIES heapProperties;
    heapProperties.Type = (desc.mode == BufferMode::Readback) ? D3D12_HEAP_TYPE_READBACK : D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Alignment = 0;
    resourceDesc.MipLevels = static_cast<UINT16>(desc.mipmaps);
    resourceDesc.Format = TranslateElementFormat(desc.format);
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    resourceDesc.Width = desc.width;
    resourceDesc.DepthOrArraySize = static_cast<UINT16>(desc.layers);

    bool passClearValue = false;
    D3D12_CLEAR_VALUE clearValue;
    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_GENERIC_READ;

    // for Static and Readback textures, the first operation performed will be copy to this texture
    if ((desc.mode == BufferMode::Static && desc.dataDesc) || (desc.mode == BufferMode::Readback))
        initialState = D3D12_RESOURCE_STATE_COPY_DEST;

    switch (desc.type)
    {
    case TextureType::Texture1D:
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        resourceDesc.Height = 1;
        break;
    case TextureType::Texture2D:
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDesc.Height = desc.height;
        break;
    case TextureType::TextureCube:
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDesc.Height = desc.height;
        break;
    case TextureType::Texture3D:
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        resourceDesc.Height = desc.height;
        resourceDesc.DepthOrArraySize = static_cast<UINT16>(desc.depth);
        break;
    }

    // if the texture is CPU-readonly we must create readback buffer resource instead of texture resource
    if (desc.mode == BufferMode::Readback)
    {
        if (desc.binding != 0)
        {
            NFE_LOG_ERROR("Readback texture can not be bound to any pipeline stage");
            return false;
        }

        if (desc.mipmaps != 1 && desc.layers != 1)
        {
            NFE_LOG_ERROR("Readback texture can contain only one layer and one mipmap");
            return false;
        }

        UINT64 requiredSize = 0;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
        gDevice->GetDevice()->GetCopyableFootprints(&resourceDesc, 0, 1, 0, &layout, nullptr, nullptr, &requiredSize);
        mRowPitch = static_cast<uint32>(layout.Footprint.RowPitch);

        // now fill with buffer description
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

        mTargetState = D3D12_RESOURCE_STATE_COPY_DEST;

        hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommittedResource(&heapProperties,
                                                                          D3D12_HEAP_FLAG_NONE,
                                                                          &resourceDesc,
                                                                          initialState,
                                                                          nullptr,
                                                                          IID_PPV_ARGS(mBuffers[0].GetPtr())));
        if (FAILED(hr))
        {
            NFE_LOG_ERROR("Failed to create readback buffer");
            return false;
        }
    }
    else
    {
        // determine formats and clear value
        mSrvFormat = TranslateElementFormat(desc.format);
        mDsvFormat = DXGI_FORMAT_UNKNOWN;

        if (desc.binding & NFE_RENDERER_TEXTURE_BIND_SHADER_WRITABLE)
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            initialState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        }

        if (desc.binding & NFE_RENDERER_TEXTURE_BIND_RENDERTARGET)
        {
            if (desc.mode != BufferMode::GPUOnly)
            {
                NFE_LOG_ERROR("Invalid resource access specified for rendertarget texture");
                return false;
            }

            clearValue.Format = mSrvFormat;
            clearValue.Color[0] = 0.0f;
            clearValue.Color[1] = 0.0f;
            clearValue.Color[2] = 0.0f;
            clearValue.Color[3] = 1.0f;
            passClearValue = true;

            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
        else if (desc.binding & NFE_RENDERER_TEXTURE_BIND_DEPTH)
        {
            if (desc.mode != BufferMode::GPUOnly)
            {
                NFE_LOG_ERROR("Invalid resource access specified for depth buffer");
                return false;
            }

            if (!TranslateDepthBufferTypes(desc.depthBufferFormat,
                                           resourceDesc.Format, mSrvFormat, mDsvFormat))
            {
                NFE_LOG_ERROR("Invalid depth buffer format");
                return false;
            }

            // texture won't be bound as shader resource
            if ((desc.binding & NFE_RENDERER_TEXTURE_BIND_SHADER) == 0)
            {
                resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
            }

            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

            clearValue.Format = mDsvFormat;
            clearValue.DepthStencil.Depth = 1.0f;
            clearValue.DepthStencil.Stencil = 0;
            passClearValue = true;

            initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        }
        else
        {
            resourceDesc.Format = TranslateElementFormat(desc.format);
            mSrvFormat = resourceDesc.Format;
            if (resourceDesc.Format == DXGI_FORMAT_UNKNOWN)
            {
                NFE_LOG_ERROR("Invalid texture format");
                return false;
            }
        }

        // create the texture resource
        hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
                                                                          &resourceDesc, initialState,
                                                                          passClearValue ? &clearValue : nullptr,
                                                                          IID_PPV_ARGS(mBuffers[0].GetPtr())));
        if (FAILED(hr))
        {
            NFE_LOG_ERROR("Failed to create texture resource");
            return false;
        }


        if (desc.binding & NFE_RENDERER_TEXTURE_BIND_SHADER)
            mTargetState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        else
            mTargetState = initialState;

        if (desc.mode == BufferMode::Static)
        {
            if (desc.dataDesc)
            {
                if (!UploadData(desc))
                    return false;
            }
            else
                NFE_LOG_WARNING("No initial data for read-only texture provided");
        }
    }

    if (desc.debugName && !SetDebugName(mBuffers[0].Get(), Common::StringView(desc.debugName)))
    {
        NFE_LOG_WARNING("Failed to set debug name");
    }


    mBuffersNum = 1;
    mCurrentBuffer = 0;
    mType = desc.type;
    mFormat = desc.format;
    mWidth = static_cast<uint16>(desc.width);
    mHeight = static_cast<uint16>(desc.height);
    mLayers = static_cast<uint16>(desc.layers);
    mMipmapsNum = static_cast<uint16>(desc.mipmaps);
    mMode = desc.mode;

    for (uint32 subresource = 0; subresource < desc.layers * desc.mipmaps; ++subresource)
        mSubresourceStates.PushBack(initialState);

    return true;
}

} // namespace Renderer
} // namespace NFE

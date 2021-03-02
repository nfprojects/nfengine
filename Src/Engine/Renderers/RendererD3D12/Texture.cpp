/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's texture
 */

#include "PCH.hpp"
#include "Texture.hpp"
#include "RendererD3D12.hpp"
#include "Translations.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/System/Timer.hpp"


namespace NFE {
namespace Renderer {

Texture::Texture()
    : Resource(D3D12_RESOURCE_STATE_COMMON)
{
}

Texture::~Texture()
{
}

bool Texture::Init(const TextureDesc& desc)
{
    HRESULT hr;

    NFE_ASSERT(desc.mode == ResourceAccessMode::Immutable || desc.mode == ResourceAccessMode::GPUOnly, "Invalid resource acces mode for a texture");

    if (desc.width < 1 || desc.width >= std::numeric_limits<uint16>::max())
    {
        NFE_LOG_ERROR("Invalid texture width (%u), max is %u");
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

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Alignment = 0;
    resourceDesc.MipLevels = static_cast<UINT16>(desc.mipmaps);
    resourceDesc.Format = TranslateElementFormat(desc.format);
    resourceDesc.SampleDesc.Count = desc.samplesNum;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    resourceDesc.Width = desc.width;
    resourceDesc.DepthOrArraySize = static_cast<UINT16>(desc.layers);

    bool passClearValue = false;
    D3D12_CLEAR_VALUE clearValue;
    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;

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

    // determine formats and clear value
    mSrvFormat = TranslateElementFormat(desc.format);
    mDsvFormat = DXGI_FORMAT_UNKNOWN;

    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_SHADER_WRITABLE)
    {
        NFE_ASSERT(desc.mode == ResourceAccessMode::GPUOnly, "Invalid access mode for writable texture");

        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        initialState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }

    if (desc.binding & NFE_RENDERER_TEXTURE_BIND_RENDERTARGET)
    {
        NFE_ASSERT(desc.mode == ResourceAccessMode::GPUOnly, "Invalid access mode for rendertarget texture");

        clearValue.Format = mSrvFormat;
        clearValue.Color[0] = desc.defaultColorClearValue[0];
        clearValue.Color[1] = desc.defaultColorClearValue[1];
        clearValue.Color[2] = desc.defaultColorClearValue[2];
        clearValue.Color[3] = desc.defaultColorClearValue[3];
        passClearValue = true;

        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }
    else if (desc.binding & NFE_RENDERER_TEXTURE_BIND_DEPTH)
    {
        NFE_ASSERT(desc.mode == ResourceAccessMode::GPUOnly, "Invalid access mode for depthbuffer texture");

        if (!TranslateDepthBufferTypes(desc.format, resourceDesc.Format, mSrvFormat, mDsvFormat))
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
        clearValue.DepthStencil.Depth = desc.defaultDepthClearValue;
        clearValue.DepthStencil.Stencil = desc.defaultStencilClearValue;
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

    UINT64 requiredSize = 0;
    gDevice->GetDevice()->GetCopyableFootprints(&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &requiredSize);
    NFE_LOG_DEBUG("Allocating texture '%s' requires %llu bytes", desc.debugName, requiredSize);

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    // create the texture resource
    hr = D3D_CALL_CHECK(gDevice->GetAllocator()->CreateResource(
        &allocationDesc,
        &resourceDesc,
        initialState,
        passClearValue ? &clearValue : nullptr,
        mAllocation.GetPtr(),
        IID_PPV_ARGS(mResource.GetPtr())));

    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create texture resource");
        return false;
    }

    mState.Set(initialState);

    if (desc.debugName && !SetDebugName(mResource.Get(), Common::StringView(desc.debugName)))
    {
        NFE_LOG_WARNING("Failed to set debug name");
    }

    mType = desc.type;
    mFormat = desc.format;
    mWidth = static_cast<uint16>(desc.width);
    mHeight = static_cast<uint16>(desc.height);
    mLayersNumOrDepth = mType == TextureType::Texture3D ? static_cast<uint16>(desc.depth) : static_cast<uint16>(desc.layers);
    mMipmapsNum = static_cast<uint8>(desc.mipmaps);
    mSamplesNum = static_cast<uint8>(desc.samplesNum);
    mMode = desc.mode;

    return true;
}

} // namespace Renderer
} // namespace NFE

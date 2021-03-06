/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's texture
 */

#include "PCH.hpp"
#include "Texture.hpp"
#include "MemoryBlock.hpp"
#include "RendererD3D12.hpp"
#include "Translations.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/System/Timer.hpp"
#include "Engine/Common/Utils/StringUtils.hpp"


namespace NFE {
namespace Renderer {

using namespace Common;

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
    resourceDesc.Format = TranslateFormat(desc.format);
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
    mSrvFormat = TranslateFormat(desc.format);
    mDsvFormat = DXGI_FORMAT_UNKNOWN;

    if ((desc.usage & TextureUsageFlag::ShaderWritableResource) == TextureUsageFlag::ShaderWritableResource)
    {
        NFE_ASSERT(desc.mode == ResourceAccessMode::GPUOnly, "Invalid access mode for writable texture");

        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        initialState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }

    if ((desc.usage & TextureUsageFlag::RenderTarget) == TextureUsageFlag::RenderTarget)
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
    else if ((desc.usage & TextureUsageFlag::DepthStencil) == TextureUsageFlag::DepthStencil)
    {
        NFE_ASSERT(desc.mode == ResourceAccessMode::GPUOnly, "Invalid access mode for depthbuffer texture");

        if (!TranslateDepthBufferTypes(desc.format, resourceDesc.Format, mSrvFormat, mDsvFormat))
        {
            NFE_LOG_ERROR("Invalid depth buffer format");
            return false;
        }

        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        // texture won't be bound as shader resource
        if ((desc.usage & TextureUsageFlag::ReadonlyShaderResource) != TextureUsageFlag::ReadonlyShaderResource)
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }

        clearValue.Format = mDsvFormat;
        clearValue.DepthStencil.Depth = desc.defaultDepthClearValue;
        clearValue.DepthStencil.Stencil = desc.defaultStencilClearValue;
        passClearValue = true;

        initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    }
    else
    {
        resourceDesc.Format = TranslateFormat(desc.format);
        mSrvFormat = resourceDesc.Format;
        if (resourceDesc.Format == DXGI_FORMAT_UNKNOWN)
        {
            NFE_LOG_ERROR("Invalid texture format");
            return false;
        }
    }

    const D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = gDevice->GetDevice()->GetResourceAllocationInfo(0u, 1u, &resourceDesc);
    NFE_LOG_DEBUG("Allocating texture '%s' requires %s aligned to %s",
        desc.debugName,
        BytesToString(allocationInfo.SizeInBytes).Str(),
        BytesToString(allocationInfo.Alignment).Str());

    // create the texture resource
    if (desc.memoryBlock)
    {
        const MemoryBlock* memoryBlock = static_cast<const MemoryBlock*>(desc.memoryBlock.Get());

        NFE_ASSERT(desc.memoryBlockOffset + allocationInfo.SizeInBytes <= memoryBlock->GetSize(),
            "Resource won't fit memory block. Resource size: %llu, memory block size: %llu, offset in block: $llu",
            allocationInfo.SizeInBytes, memoryBlock->GetSize(), desc.memoryBlockOffset);

        hr = D3D_CALL_CHECK(gDevice->GetAllocator()->CreateAliasingResource(
            memoryBlock->GetAllocation(),
            desc.memoryBlockOffset,
            &resourceDesc,
            initialState,
            passClearValue ? &clearValue : nullptr,
            IID_PPV_ARGS(mResource.GetPtr())));
    }
    else
    {
        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        hr = D3D_CALL_CHECK(gDevice->GetAllocator()->CreateResource(
            &allocationDesc,
            &resourceDesc,
            initialState,
            passClearValue ? &clearValue : nullptr,
            mAllocation.GetPtr(),
            IID_PPV_ARGS(mResource.GetPtr())));
    }

    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create texture resource");
        return false;
    }

    mState.Set(initialState);

    if (desc.debugName && !SetDebugName(mResource.Get(), StringView(desc.debugName)))
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
    mMemoryBlock = desc.memoryBlock;

    return true;
}

} // namespace Renderer
} // namespace NFE

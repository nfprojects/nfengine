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

static void CreateTextureSRV(const Texture* tex, const TextureView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = tex->GetSrvFormat();

    if (tex->GetType() == TextureType::Texture1D)
    {
        if (tex->GetLayersNum() > 1)
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            srvDesc.Texture1DArray.MostDetailedMip = view.baseMip;
            srvDesc.Texture1DArray.FirstArraySlice = view.baseLayer;
            srvDesc.Texture1DArray.MipLevels = view.numMips;
            srvDesc.Texture1DArray.ArraySize = view.numLayers;
            srvDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
        }
        else
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            srvDesc.Texture1D.MostDetailedMip = view.baseMip;
            srvDesc.Texture1D.MipLevels = view.numMips;
            srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
        }
    }
    else if (tex->GetType() == TextureType::Texture2D)
    {
        if (tex->GetSamplesNum() > 1)
        {
            if (tex->GetLayersNum() > 1)
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                srvDesc.Texture2DMSArray.FirstArraySlice = view.baseLayer;
                srvDesc.Texture2DMSArray.ArraySize = view.numLayers;
            }
            else
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            }
        }
        else
        {
            if (tex->GetLayersNum() > 1)
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.MostDetailedMip = view.baseMip;
                srvDesc.Texture2DArray.FirstArraySlice = view.baseLayer;
                srvDesc.Texture2DArray.MipLevels = view.numMips;
                srvDesc.Texture2DArray.ArraySize = view.numLayers;
                srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
            }
            else
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip = view.baseMip;
                srvDesc.Texture2D.MipLevels = view.numMips;
                srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
                srvDesc.Texture2D.PlaneSlice = 0;
            }
        }
    }
    else if (tex->GetType() == TextureType::TextureCube)
    {
        // TODO cube texture array
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip = view.baseMip;
        srvDesc.TextureCube.MipLevels = view.numMips;
        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    }
    else if (tex->GetType() == TextureType::Texture3D)
    {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MostDetailedMip = view.baseMip;
        srvDesc.Texture3D.MipLevels = view.numMips;
        srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
    }
    else
    {
        NFE_FATAL("Invalid texture type");
        return;
    }

    gDevice->GetDevice()->CreateShaderResourceView(tex->GetD3DResource(), &srvDesc, descriptorHandle);
}

static void CreateTextureUAV(const Texture* tex, const TextureView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = tex->GetSrvFormat();

    if (tex->GetType() == TextureType::Texture1D)
    {
        if (tex->GetLayersNum() > 1)
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            uavDesc.Texture1DArray.FirstArraySlice = view.baseLayer;
            uavDesc.Texture1DArray.MipSlice = view.baseMip;
            uavDesc.Texture1DArray.ArraySize = view.numLayers;
        }
        else
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
            uavDesc.Texture1D.MipSlice = view.baseMip;
        }
    }
    else if (tex->GetType() == TextureType::Texture2D)
    {
        if (tex->GetLayersNum() > 1)
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            uavDesc.Texture2DArray.FirstArraySlice = view.baseLayer;
            uavDesc.Texture2DArray.MipSlice = view.baseMip;
            uavDesc.Texture2DArray.ArraySize = view.numLayers;
            uavDesc.Texture2DArray.PlaneSlice = 0;
        }
        else
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice = view.baseMip;
            uavDesc.Texture2D.PlaneSlice = 0;
        }
    }
    else if (tex->GetType() == TextureType::Texture3D)
    {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uavDesc.Texture3D.FirstWSlice = view.baseLayer;
        uavDesc.Texture3D.MipSlice = view.baseMip;
        uavDesc.Texture3D.WSize = view.numLayers;
    }
    else
    {
        NFE_FATAL("Invalid texture type");
        return;
    }

    gDevice->GetDevice()->CreateUnorderedAccessView(tex->GetD3DResource(), nullptr, &uavDesc, descriptorHandle);
}

DescriptorID Texture::GetDescriptor(DescriptorType type, const TextureView& view)
{
    // check if descriptor was already created
    for (const auto& iter : mDescriptors)
    {
        if (iter.type == type && iter.view == view)
        {
            return iter.descriptor;
        }
    }

    // create missing descriptor

    HeapAllocator& stagingHeapAllocator = gDevice->GetCbvSrvUavHeapStagingAllocator();
    const DescriptorID descriptorID = stagingHeapAllocator.Allocate(1);

    if (type == DescriptorType::SRV)
    {
        CreateTextureSRV(this, view, stagingHeapAllocator.GetCpuHandle(descriptorID));
    }
    else if (type == DescriptorType::UAV)
    {
        CreateTextureUAV(this, view, stagingHeapAllocator.GetCpuHandle(descriptorID));
    }
    else
    {
        NFE_FATAL("Invalid descriptor type for texture");
    }

    mDescriptors.PushBack(DescriptorInfo{ type, view, descriptorID });

    return descriptorID;
}

void Texture::DeleteDescriptors()
{
    for (const auto& iter : mDescriptors)
    {
        gDevice->GetCbvSrvUavHeapStagingAllocator().Free(iter.descriptor, 1);
    }
    mDescriptors.Clear();
}

} // namespace Renderer
} // namespace NFE

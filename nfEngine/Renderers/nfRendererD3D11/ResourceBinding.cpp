/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's renderer's shader resource binding
 */

#include "PCH.hpp"
#include "ResourceBinding.hpp"
#include "Texture.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"
#include "RendererD3D11.hpp"

#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

bool ResourceBindingSet::IsBindingOverlapping(const ResourceBindingDesc& bindingDesc) const
{
    for (const auto& desc : mBindings)
    {
        if (desc.slot == bindingDesc.slot && desc.resourceType == bindingDesc.resourceType)
        {
            return true;
        }
    }
    return false;
}

bool ResourceBindingSet::IsBindingSetOverlapping(const ResourceBindingSet* set) const
{
    for (const auto& desc : set->mBindings)
    {
        if (IsBindingOverlapping(desc))
        {
            if (mShaderVisibility == ShaderType::All || set->mShaderVisibility == ShaderType::All ||
                mShaderVisibility == set->mShaderVisibility)
                return true;
        }
    }
    return false;
}

bool ResourceBindingSet::Init(const ResourceBindingSetDesc& desc)
{
    if (desc.numBindings == 0)
    {
        NFE_LOG_ERROR("Binding set can not be empty");
        return false;
    }

    if (desc.shaderVisibility != ShaderType::Vertex &&
        desc.shaderVisibility != ShaderType::Hull &&
        desc.shaderVisibility != ShaderType::Domain &&
        desc.shaderVisibility != ShaderType::Geometry &&
        desc.shaderVisibility != ShaderType::Pixel &&
        desc.shaderVisibility != ShaderType::Compute &&
        desc.shaderVisibility != ShaderType::All)
    {
        NFE_LOG_ERROR("Invalid shader visibility");
        return false;
    }


    mBindings.Reserve(desc.numBindings);

    for (uint32 i = 0; i < desc.numBindings; ++i)
    {
        const ResourceBindingDesc& bindingDesc = desc.resourceBindings[i];

        if (bindingDesc.resourceType != ShaderResourceType::CBuffer &&
            bindingDesc.resourceType != ShaderResourceType::Texture &&
            bindingDesc.resourceType != ShaderResourceType::StructuredBuffer &&
            bindingDesc.resourceType != ShaderResourceType::WritableTexture &&
            bindingDesc.resourceType != ShaderResourceType::WritableStructuredBuffer)
        {
            NFE_LOG_ERROR("Invalid shader resource type at binding %i", i);
            return false;
        }

        mBindings.PushBack(desc.resourceBindings[i]);
    }

    mShaderVisibility = desc.shaderVisibility;
    return true;
}

bool ResourceBindingLayout::Init(const ResourceBindingLayoutDesc& desc)
{
    mBindingSets.Reserve(desc.numBindingSets);

    for (uint32 i = 0; i < desc.numBindingSets; ++i)
    {
        InternalResourceBindingSetPtr bindingSet = Common::StaticCast<ResourceBindingSet>(desc.bindingSets[i]);
        if (bindingSet == nullptr)
        {
            NFE_LOG_ERROR("Invalid binding set");
            return false;
        }

        for (uint32 j = 0; j < i; ++j)
        {
            if (mBindingSets[j] == bindingSet)
            {
                NFE_LOG_ERROR("Same binding sets (%u and %u) can't be reused in a binding layout", j, i);
                return false;
            }

            if (bindingSet->IsBindingSetOverlapping(mBindingSets[j].Get()))
            {
                NFE_LOG_ERROR("Resource binding slots are overlapping (sets %u and %u)", j, i);
                return false;
            }
        }

        mBindingSets.PushBack(bindingSet);
    }

    for (uint32 i = 0; i < desc.numVolatileCBuffers; ++i)
    {
        // TODO check if shader slots are not overlapping
        mVolatileCBuffers.PushBack(desc.volatileCBuffers[i]);
    }

    return true;
}

bool ResourceBindingInstance::Init(const ResourceBindingSetPtr& bindingSet)
{
    mBindingSet = Common::StaticCast<ResourceBindingSet>(bindingSet);
    if (bindingSet == nullptr)
    {
        NFE_LOG_ERROR("Invalid binding set");
        return false;
    }

    mViews.Resize(mBindingSet->mBindings.Size());
    mCBuffers.Resize(mBindingSet->mBindings.Size());
    return true;
}

bool ResourceBindingInstance::WriteTextureView(uint32 slot, const TexturePtr& texture)
{
    if (slot >= mBindingSet->mBindings.Size())
    {
        NFE_LOG_ERROR("Invalid binding set slot %u (there are %u slots)", slot, mBindingSet->mBindings.Size());
        return false;
    }

    const Texture* tex = dynamic_cast<Texture*>(texture.Get());
    if (!tex)
    {
        NFE_LOG_ERROR("Invalid texture");
        return false;
    }


    // fill up the SRV descriptor

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
    ZeroMemory(&srvd, sizeof(srvd));
    srvd.Format = tex->mSrvFormat;

    if (tex->mType == TextureType::Texture1D)
    {
        if (tex->mLayers == 1) // single-layered, 1D texture
        {
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
            srvd.Texture1D.MipLevels = tex->mMipmaps;
            srvd.Texture1D.MostDetailedMip = 0;
        }
        else // multi-layered, 1D texture
        {
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
            srvd.Texture1DArray.ArraySize = tex->mLayers;
            srvd.Texture1DArray.FirstArraySlice = 0;
            srvd.Texture1DArray.MipLevels = tex->mMipmaps;
            srvd.Texture1DArray.MostDetailedMip = 0;
        }
    }
    else if (tex->mType == TextureType::Texture2D)
    {
        if (tex->mSamples == 1)
        {
            if (tex->mLayers == 1) // single-layered, single-sampled, 2D texture
            {
                srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvd.Texture2D.MipLevels = tex->mMipmaps;
                srvd.Texture2D.MostDetailedMip = 0;
            }
            else // multi-layered, single-sampled, 2D texture
            {
                srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                srvd.Texture2DArray.ArraySize = tex->mLayers;
                srvd.Texture2DArray.FirstArraySlice = 0;
                srvd.Texture2DArray.MipLevels = tex->mMipmaps;
                srvd.Texture2DArray.MostDetailedMip = 0;
            }
        }
        else
        {
            if (tex->mLayers == 1) // single-layered, multi-sampled, 2D texture
            {
                srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
            }
            else // multi-layered, multi-sampled, 2D texture
            {
                srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
                srvd.Texture2DMSArray.ArraySize = tex->mLayers;
                srvd.Texture2DMSArray.FirstArraySlice = 0;
            }
        }
    }
    else if (tex->mType == TextureType::TextureCube)
    {
        if (tex->mLayers == 6) // single-layered, Cube texture
        {
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            srvd.TextureCube.MipLevels = tex->mMipmaps;
            srvd.TextureCube.MostDetailedMip = 0;
        }
        else // multi-layered, Cube texture
        {
            srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
            srvd.TextureCubeArray.MipLevels = tex->mMipmaps;
            srvd.TextureCubeArray.MostDetailedMip = 0;
            srvd.TextureCubeArray.First2DArrayFace = 0;
            srvd.TextureCubeArray.NumCubes = tex->mLayers / 6;
        }
    }
    else if (tex->mType == TextureType::Texture3D) // 3D texture
    {
        srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
        srvd.Texture3D.MipLevels = tex->mMipmaps;
        srvd.Texture3D.MostDetailedMip = 0;
    }

    D3DPtr<ID3D11ShaderResourceView> srv;
    HRESULT hr = D3D_CALL_CHECK(gDevice->Get()->CreateShaderResourceView(tex->mTextureGeneric, &srvd, srv.GetPtr()));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create Shader Resource View");
        return false;
    }

    mViews[slot] = srv.Release();
    return true;
}

bool ResourceBindingInstance::WriteCBufferView(uint32 slot, const BufferPtr& buffer)
{
    if (slot >= mBindingSet->mBindings.Size())
    {
        NFE_LOG_ERROR("Invalid binding set slot %u (there are %u slots)", slot, mBindingSet->mBindings.Size());
        return false;
    }

    const Buffer* buf = dynamic_cast<Buffer*>(buffer.Get());
    if (!buf || !buf->mBuffer)
    {
        NFE_LOG_ERROR("Invalid constant buffer");
        return false;
    }

    if (buf->mType != BufferType::Constant)
    {
        NFE_LOG_ERROR("Not a constant buffer");
        return false;
    }

    mCBuffers[slot] = buf->mBuffer.Get();
    return true;
}

bool ResourceBindingInstance::WriteWritableTextureView(uint32 slot, const TexturePtr& texture)
{
    if (slot >= mBindingSet->mBindings.Size())
    {
        NFE_LOG_ERROR("Invalid binding set slot %u (there are %u slots)", slot, mBindingSet->mBindings.Size());
        return false;
    }

    const Texture* tex = dynamic_cast<Texture*>(texture.Get());
    if (!tex)
    {
        NFE_LOG_ERROR("Invalid texture");
        return false;
    }

    // fill up the UAV descriptor

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
    ZeroMemory(&uavd, sizeof(uavd));
    uavd.Format = tex->mSrvFormat;

    if (tex->mType == TextureType::Texture1D)
    {
        if (tex->mLayers == 1) // single-layered, 1D texture
        {
            uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
            uavd.Texture1D.MipSlice = 0;
        }
        else // multi-layered, 1D texture
        {
            uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
            uavd.Texture1DArray.ArraySize = tex->mLayers;
            uavd.Texture1DArray.FirstArraySlice = 0;
            uavd.Texture1DArray.MipSlice = 0;
        }
    }
    else if (tex->mType == TextureType::Texture2D)
    {
        if (tex->mLayers == 1) // single-layered, single-sampled, 2D texture
        {
            uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavd.Texture2D.MipSlice = 0;
        }
        else // multi-layered, single-sampled, 2D texture
        {
            uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
            uavd.Texture2DArray.ArraySize = tex->mLayers;
            uavd.Texture2DArray.FirstArraySlice = 0;
            uavd.Texture2DArray.MipSlice = 0;
        }
    }
    else if (tex->mType == TextureType::Texture3D) // 3D texture
    {
        uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
        uavd.Texture3D.FirstWSlice = 0;
        uavd.Texture3D.MipSlice = 0;
        uavd.Texture3D.WSize = tex->mLayers;
    }

    D3DPtr<ID3D11UnorderedAccessView> uav;
    HRESULT hr = D3D_CALL_CHECK(gDevice->Get()->CreateUnorderedAccessView(tex->mTextureGeneric, &uavd, uav.GetPtr()));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create Unordered Access View");
        return false;
    }

    mViews[slot] = uav.Release();
    return true;
}

} // namespace Renderer
} // namespace NFE

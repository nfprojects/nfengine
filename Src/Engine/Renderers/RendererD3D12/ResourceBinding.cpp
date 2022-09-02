/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's shader resource binding
 */

#include "PCH.hpp"
#include "ResourceBinding.hpp"
#include "Texture.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"
#include "Shader.hpp"
#include "Translations.hpp"
#include "RendererD3D12.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/Math/Math.hpp"


namespace NFE {
namespace Renderer {

void CreateTextureSRV(const Texture* tex, const TextureView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
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

void CreateTextureUAV(const Texture* tex, const TextureView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
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

void CreateBufferSRV(const Buffer* buf, const BufferView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
    const uint32 structSize = buf->GetStructSize();
    const uint32 maxElements = buf->GetSize() / structSize;

    const uint32 numElements = (view.numElements == UINT32_MAX) ? maxElements : view.numElements;

    NFE_ASSERT(view.firstElement < maxElements, "Buffer first element out of bounds");
    NFE_ASSERT(view.firstElement + numElements <= maxElements, "Buffer range out of bounds");

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.FirstElement = view.firstElement;
    srvDesc.Buffer.NumElements = numElements;
    srvDesc.Buffer.StructureByteStride = structSize; // TODO custom stride
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    gDevice->GetDevice()->CreateShaderResourceView(buf->GetD3DResource(), &srvDesc, descriptorHandle);
}

void CreateBufferUAV(const Buffer* buf, const BufferView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
    const uint32 structSize = buf->GetStructSize();
    const uint32 maxElements = buf->GetSize() / structSize;

    const uint32 numElements = (view.numElements == UINT32_MAX) ? maxElements : view.numElements;

    NFE_ASSERT(view.firstElement < maxElements, "Buffer first element out of bounds");
    NFE_ASSERT(view.firstElement + numElements <= maxElements, "Buffer range out of bounds");

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.Buffer.FirstElement = view.firstElement;
    uavDesc.Buffer.NumElements = numElements;
    uavDesc.Buffer.StructureByteStride = structSize; // TODO custom stride
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

    gDevice->GetDevice()->CreateUnorderedAccessView(buf->GetD3DResource(), nullptr, &uavDesc, descriptorHandle);
}

} // namespace Renderer
} // namespace NFE

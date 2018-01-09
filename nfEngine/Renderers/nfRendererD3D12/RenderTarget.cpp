/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's render target
 */

#include "PCH.hpp"
#include "RenderTarget.hpp"
#include "Texture.hpp"
#include "RendererD3D12.hpp"
#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

RenderTarget::RenderTarget()
    : mDSV(static_cast<uint32>(-1))
    , mDepthTexture(nullptr)
    , mDepthTextureSubresource(0)
{
}

RenderTarget::~RenderTarget()
{
    HeapAllocator& allocator = gDevice->GetRtvHeapAllocator();

    for (size_t i = 0; i < 2; ++i)
    {
        for (uint32 offset : mRTVs[i])
            allocator.Free(offset, 1);
    }

    HeapAllocator& dsvAllocator = gDevice->GetDsvHeapAllocator();
    if (mDSV != -1)
        dsvAllocator.Free(mDSV, 1);

    gDevice->WaitForGPU();
}

void RenderTarget::GetDimensions(int& width, int& height)
{
    width = mTargets[0].texture->GetWidth();
    height = mTargets[0].texture->GetHeight();
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    HeapAllocator& rtvAllocator = gDevice->GetRtvHeapAllocator();

    for (unsigned int i = 0; i < desc.numTargets; ++i)
    {
        Texture* tex = dynamic_cast<Texture*>(desc.targets[i].texture.Get());
        if (tex == nullptr)
        {
            NFE_LOG_ERROR("Invalid target texture at index %u", i);
            return false;
        }

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = tex->mSrvFormat;
        switch (tex->mType)
        {
        case TextureType::Texture1D:
            if (tex->mLayers > 1)
            {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                rtvDesc.Texture1DArray.MipSlice = desc.targets[i].level;
                rtvDesc.Texture1DArray.FirstArraySlice = desc.targets[i].layer;
                rtvDesc.Texture1DArray.ArraySize = 1;
                rtvDesc.Texture2DArray.PlaneSlice = 0;
            }
            else
            {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                rtvDesc.Texture1D.MipSlice = desc.targets[i].level;
            }
            break;
        case TextureType::Texture2D:
        case TextureType::TextureCube:
            if (tex->mLayers == 1)
            {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;
                rtvDesc.Texture2D.PlaneSlice = 0;
            }
            else if (tex->mLayers > 1)
            {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvDesc.Texture2DArray.MipSlice = desc.targets[i].level;
                rtvDesc.Texture2DArray.FirstArraySlice = desc.targets[i].layer;
                rtvDesc.Texture2DArray.ArraySize = 1;
                rtvDesc.Texture2DArray.PlaneSlice = 0;
            }
            else
            {
                NFE_LOG_ERROR("Unsupported texture type");
                return false;
            }

            break;
            // TODO TextureType::Texture3D
            // TODO multisampled textures

        default:
            NFE_LOG_ERROR("Unsupported texture type for render target");
            return false;
        }

        // Create a RTV for each frame
        for (UINT n = 0; n < tex->mBuffersNum; n++)
        {
            uint32 offset = rtvAllocator.Allocate(1);
            if (offset == UINT32_MAX)
                return false;

            D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvAllocator.GetCpuHandle();
            handle.ptr += rtvAllocator.GetDescriptorSize() * offset;
            gDevice->mDevice->CreateRenderTargetView(tex->mBuffers[n].Get(), &rtvDesc, handle);
            mRTVs[n].push_back(offset);
        }

        Target targetInfo;
        targetInfo.texture = Common::DynamicCast<Texture>(desc.targets[i].texture);
        targetInfo.subresource = desc.targets[i].level + desc.targets[i].layer * tex->GetMipmapsNum();
        mTargets.push_back(targetInfo);
    }

    if (desc.depthBuffer)
    {
        HeapAllocator& allocator = gDevice->GetDsvHeapAllocator();

        Texture* tex = dynamic_cast<Texture*>(desc.depthBuffer.Get());
        if (tex == nullptr)
        {
            NFE_LOG_ERROR("Invalid texture for depth buffer");
            return false;
        }

        mDSV = allocator.Allocate(1);
        if (mDSV == -1)
            return false;

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = tex->mDsvFormat;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

        switch (tex->mType)
        {
        case TextureType::Texture1D:
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
            dsvDesc.Texture1D.MipSlice = 0;
            break;
        case TextureType::Texture2D:
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;
            break;
        // TODO multisampled and multilayered textures
        default:
            NFE_LOG_ERROR("Feature not implemented");
            return false;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE handle = allocator.GetCpuHandle();
        handle.ptr += allocator.GetDescriptorSize() * mDSV;
        gDevice->mDevice->CreateDepthStencilView(tex->mBuffers[0].Get(), &dsvDesc, handle);

        mDepthTexture = Common::DynamicCast<Texture>(desc.depthBuffer);
        mDepthTextureSubresource = 0; // TODO: selectable mipmap and texture layer in the interface
    }

    return true;
}

} // namespace Renderer
} // namespace NFE

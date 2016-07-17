/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's render target
 */

#include "PCH.hpp"
#include "RenderTarget.hpp"
#include "Texture.hpp"
#include "RendererD3D12.hpp"
#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {

RenderTarget::RenderTarget()
    : mWidth(0)
    , mHeight(0)
    , mDSV(static_cast<uint32>(-1))
    , mDepthTexture(nullptr)
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
    width = mWidth;
    height = mHeight;
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    HeapAllocator& rtvAllocator = gDevice->GetRtvHeapAllocator();

    for (unsigned int i = 0; i < desc.numTargets; ++i)
    {
        Texture* tex = dynamic_cast<Texture*>(desc.targets[i].texture);
        if (tex == nullptr)
        {
            LOG_ERROR("Invalid target texture at index %u", i);
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
            gDevice->mDevice->CreateRenderTargetView(tex->mBuffers[n].get(), nullptr, handle);
            mRTVs[n].push_back(offset);
        }

        mTextures.push_back(tex);
    }

    if (desc.depthBuffer)
    {
        HeapAllocator& allocator = gDevice->GetDsvHeapAllocator();

        Texture* tex = dynamic_cast<Texture*>(desc.depthBuffer);
        if (tex == nullptr)
        {
            LOG_ERROR("Invalid texture for depth buffer");
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
            LOG_ERROR("Feature not implemented");
            return false;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE handle = allocator.GetCpuHandle();
        handle.ptr += allocator.GetDescriptorSize() * mDSV;
        gDevice->mDevice->CreateDepthStencilView(tex->mBuffers[0].get(), &dsvDesc, handle);

        mDepthTexture = tex;
    }

    return true;
}

} // namespace Renderer
} // namespace NFE

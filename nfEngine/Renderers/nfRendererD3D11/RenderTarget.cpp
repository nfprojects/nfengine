/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's render target
 */

#include "PCH.hpp"
#include "RenderTarget.hpp"
#include "Texture.hpp"
#include "RendererD3D11.hpp"

#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

RenderTarget::RenderTarget()
    : mDepthBuffer(nullptr)
    , mWidth(0)
    , mHeight(0)
{
}

void RenderTarget::GetDimensions(int& width, int& height)
{
    width = mWidth;
    height = mHeight;
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    HRESULT hr;

    if (desc.numTargets > MAX_RENDER_TARGETS)
    {
        NFE_LOG_ERROR("Too many targets (got %u, max is %u", desc.numTargets, MAX_RENDER_TARGETS);
        return false;
    }

    mRTVs.Reserve(desc.numTargets);
    for (uint32 i = 0; i < desc.numTargets; ++i)
    {
        Texture* tex = dynamic_cast<Texture*>(desc.targets[i].texture.Get());

        if (tex == nullptr)
        {
            NFE_LOG_ERROR("Invalid texture pointer");
            mRTVs.Clear();
            return false;
        }

        if (i == 0)
        {
            mWidth = tex->mWidth;
            mHeight = tex->mHeight;
        }
        else if (mWidth != tex->mWidth || mHeight != tex->mHeight)
        {
            NFE_LOG_ERROR("Render target's texture dimensions do not match");
            return false;
        }

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = DXGI_FORMAT_UNKNOWN;

        switch (tex->mType)
        {
        case TextureType::Texture1D:
            if (tex->mLayers > 1)
            {
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
                rtvDesc.Texture1DArray.MipSlice = desc.targets[i].level;
                rtvDesc.Texture1DArray.FirstArraySlice = desc.targets[i].layer;
                rtvDesc.Texture1DArray.ArraySize = 1;
            }
            else
            {
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
                rtvDesc.Texture1D.MipSlice = desc.targets[i].level;
            }
            break;
        case TextureType::Texture2D:
        case TextureType::TextureCube:
            if (tex->mLayers == 1 && tex->mSamples == 1)
            {
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;
            }
            else if (tex->mLayers > 1 && tex->mSamples == 1)
            {
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvDesc.Texture2DArray.MipSlice = desc.targets[i].level;
                rtvDesc.Texture2DArray.FirstArraySlice = desc.targets[i].layer;
                rtvDesc.Texture2DArray.ArraySize = 1;
            }
            else if (tex->mLayers == 1 && tex->mSamples > 1)
            {
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
            }
            else if (tex->mLayers > 1 && tex->mSamples > 1)
            {
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                rtvDesc.Texture2DMSArray.FirstArraySlice = desc.targets[i].layer;
                rtvDesc.Texture2DMSArray.ArraySize = 1;
            }
            else
            {
                NFE_LOG_ERROR("Unsupported texture type");
                mRTVs.Clear();
                return false;
            }

            break;
            // TODO TextureType::Texture3D
        default:
            {
                NFE_LOG_ERROR("Unsupported texture type for render target");
                mRTVs.Clear();
                return false;
            }
        }

        ID3D11RenderTargetView* rtv;
        hr = D3D_CALL_CHECK(gDevice->mDevice->CreateRenderTargetView(tex->mTexture2D, &rtvDesc, &rtv));
        if (FAILED(hr))
        {
            NFE_LOG_ERROR("Failed to create render target view");
            mRTVs.Clear();
            return false;
        }

        if (gDevice->IsDebugLayerEnabled() && desc.debugName)
        {
            /// set debug name
            Common::String bufferName = "NFE::Renderer::RenderTarget \"";
            if (desc.debugName)
                bufferName += desc.debugName;
            bufferName += '"';
            D3D_CALL_CHECK(rtv->SetPrivateData(WKPDID_D3DDebugObjectName, bufferName.Length(), bufferName.Str()));
        }

        mRTVs.PushBack(D3DPtr<ID3D11RenderTargetView>(rtv));
    }

    if (desc.depthBuffer != nullptr)
    {
        mDepthBuffer = dynamic_cast<Texture*>(desc.depthBuffer.Get());
    }

    return true;
}

} // namespace Renderer
} // namespace NFE

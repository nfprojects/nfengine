/**
 * @file    RenderTarget.cpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's render target
 */

#include "stdafx.hpp"
#include "RendererD3D11.hpp"

namespace NFE {
namespace Renderer {

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    HRESULT hr;

    if (desc.numTargets > MAX_RENDER_TARGETS)
    {
        Log("Too many targets");
        return false;
    }

    mRTVs.reserve(desc.numTargets);
    for (unsigned int i = 0; i < desc.numTargets; ++i)
    {
        Texture* tex = dynamic_cast<Texture*>(desc.targets[i].texture);

        if (tex == nullptr)
        {
            Log("Invalid texture pointer");
            mRTVs.clear();
            return false;
        }
  
        // TODO: provide proper D3D11_RENDER_TARGET_VIEW_DESC
        ID3D11RenderTargetView* rtv;
        hr = gDevice->mDevice->CreateRenderTargetView(tex->mTexture2D, NULL, &rtv);
        if (FAILED(hr))
        {
            Log("Failed to create render target view");
            mRTVs.clear();
            return false;
        }


        mRTVs.emplace_back(rtv);
    }


    if (desc.depthBuffer != nullptr)
    {
        Texture* tex = dynamic_cast<Texture*>(desc.depthBuffer->texture);

        // TODO: provide proper D3D11_DEPTH_STENCIL_VIEW_DESC
        hr = gDevice->mDevice->CreateDepthStencilView(tex->mTexture2D, NULL, &mDSV);
        if (FAILED(hr))
        {
            Log("Failed to create depth-stencil view");
            mRTVs.clear();
            return false;
        }
    }

    return true;
}

} // namespace Renderer
} // namespace NFE

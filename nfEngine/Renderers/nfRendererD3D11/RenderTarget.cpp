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

    for (int i = 0; i < desc.numTargets; ++i)
    {
        Texture* tex = dynamic_cast<Texture*>(desc.targets[i].texture);

        if (tex == nullptr)
        {
            Log("Invalid texture pointer");
            return false;
        }

        ID3D11RenderTargetView* rtv;
        hr = gDevice->mDevice->CreateRenderTargetView(tex->mTexture2D, NULL, &rtv);
        if (FAILED(hr))
        {
            Log("Failed to create render target view");
            return false;
        }

        mRTVs.emplace_back(rtv);
    }

    // TODO: Depth Stencil View

    return true;
}

} // namespace Renderer
} // namespace NFE

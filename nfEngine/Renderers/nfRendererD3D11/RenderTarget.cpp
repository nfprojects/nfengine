/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's render target
 */

#include "PCH.hpp"
#include "RenderTarget.hpp"
#include "Texture.hpp"
#include "RendererD3D11.hpp"
#include "../../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

RenderTarget::RenderTarget()
{
    mDepthBuffer = nullptr;
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    HRESULT hr;

    if (desc.numTargets > MAX_RENDER_TARGETS)
    {
        LOG_ERROR("Too many targets (got %u, max is %u", desc.numTargets, MAX_RENDER_TARGETS);
        return false;
    }

    mRTVs.reserve(desc.numTargets);
    for (unsigned int i = 0; i < desc.numTargets; ++i)
    {
        Texture* tex = dynamic_cast<Texture*>(desc.targets[i].texture);

        if (tex == nullptr)
        {
            LOG_ERROR("Invalid texture pointer");
            mRTVs.clear();
            return false;
        }

        // TODO: provide proper D3D11_RENDER_TARGET_VIEW_DESC
        ID3D11RenderTargetView* rtv;
        hr = D3D_CALL_CHECK(gDevice->mDevice->CreateRenderTargetView(tex->mTexture2D, NULL, &rtv));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create render target view");
            mRTVs.clear();
            return false;
        }

#ifdef D3D_DEBUGGING
        /// set debug name
        std::string bufferName = "NFE::Renderer::RenderTarget \"";
        if (desc.debugName)
            bufferName += desc.debugName;
        bufferName += '"';
        rtv->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(bufferName.length()),
                            bufferName.c_str());
#endif // D3D_DEBUGGING

        mRTVs.emplace_back(rtv);
    }

    if (desc.depthBuffer != nullptr)
    {
        mDepthBuffer = dynamic_cast<Texture*>(desc.depthBuffer);
    }

    return true;
}

} // namespace Renderer
} // namespace NFE

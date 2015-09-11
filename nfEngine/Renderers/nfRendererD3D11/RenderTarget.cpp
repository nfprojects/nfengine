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

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = DXGI_FORMAT_UNKNOWN;

        switch (tex->type)
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
            if (tex->mLayers > 1)
            {
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvDesc.Texture2DArray.MipSlice = desc.targets[i].level;
                rtvDesc.Texture2DArray.FirstArraySlice = desc.targets[i].layer;
                rtvDesc.Texture2DArray.ArraySize = 1;
            }
            else
            {
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;
            }
            break;
            // TODO TextureType::Texture3D
        default:
            {
                LOG_ERROR("Unsupported texture typ");
                mRTVs.clear();
                return false;
            }
        }

        ID3D11RenderTargetView* rtv;
        hr = D3D_CALL_CHECK(gDevice->mDevice->CreateRenderTargetView(tex->mTexture2D, &rtvDesc, &rtv));
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

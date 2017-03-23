/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's texture sampler
 */

#include "PCH.hpp"
#include "Sampler.hpp"
#include "RendererD3D11.hpp"
#include "Translations.hpp"

namespace NFE {
namespace Renderer {

bool Sampler::Init(const SamplerDesc& desc)
{
    D3D11_SAMPLER_DESC sd;
    sd.AddressU = TranslateTextureWrapMode(desc.wrapModeU);
    sd.AddressV = TranslateTextureWrapMode(desc.wrapModeV);
    sd.AddressW = TranslateTextureWrapMode(desc.wrapModeW);
    sd.MaxAnisotropy = desc.maxAnisotropy;
    sd.Filter = TranslateFilterType(desc.minFilter, desc.magFilter, desc.compare,
                                    desc.maxAnisotropy > 1);
    sd.ComparisonFunc = TranslateComparisonFunc(desc.compareFunc);
    sd.MipLODBias = desc.mipmapBias;
    sd.MinLOD = desc.minMipmap;
    sd.MaxLOD = desc.maxMipmap;

    if (desc.borderColor)
    {
        sd.BorderColor[0] = desc.borderColor[0];
        sd.BorderColor[1] = desc.borderColor[1];
        sd.BorderColor[2] = desc.borderColor[2];
        sd.BorderColor[3] = desc.borderColor[3];
    }
    else
    {
        sd.BorderColor[0] = sd.BorderColor[1] = sd.BorderColor[2] = sd.BorderColor[3] = 0.0f;
    }

    HRESULT hr = D3D_CALL_CHECK(gDevice->Get()->CreateSamplerState(&sd, &mSamplerState));
    if (FAILED(hr))
        return false;

    if (gDevice->IsDebugLayerEnabled() && desc.debugName)
    {
        /// set debug name
        String bufferName = "NFE::Renderer::Sampler \"";
        if (desc.debugName)
            bufferName += desc.debugName;
        bufferName += '"';
        D3D_CALL_CHECK(mSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName,
                                                     static_cast<UINT>(bufferName.length()), bufferName.Str()));
    }

    return true;
}

} // namespace Renderer
} // namespace NFE

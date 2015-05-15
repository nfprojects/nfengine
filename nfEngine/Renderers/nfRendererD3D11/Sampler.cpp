/**
 * @file    Sampler.cpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's texture sampler
 */

#include "PCH.hpp"
#include "RendererD3D11.hpp"

namespace NFE {
namespace Renderer {

bool Sampler::Init(const SamplerDesc& desc)
{
    D3D11_SAMPLER_DESC sd;
    sd.AddressU = TranslateTextureWrapMode(desc.wrapModeU);
    sd.AddressV = TranslateTextureWrapMode(desc.wrapModeV);
    sd.AddressW = TranslateTextureWrapMode(desc.wrapModeW);
    sd.MaxAnisotropy = desc.maxAnisotropy;
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; // TODO
    sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sd.MipLODBias = 0.0f;
    sd.MinLOD = -D3D11_FLOAT32_MAX;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    sd.BorderColor[0] = 0.0f;
    sd.BorderColor[1] = 0.0f;
    sd.BorderColor[2] = 0.0f;
    sd.BorderColor[3] = 0.0f;

    HRESULT hr = D3D_CALL_CHECK(gDevice->Get()->CreateSamplerState(&sd, &mSamplerState));
    return SUCCEEDED(hr);
}

} // namespace Renderer
} // namespace NFE

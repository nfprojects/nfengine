/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's texture sampler
 */

#include "PCH.hpp"
#include "Sampler.hpp"
#include "RendererD3D12.hpp"
#include "Translations.hpp"

namespace NFE {
namespace Renderer {

bool Sampler::Init(const SamplerDesc& desc)
{
    D3D12_SAMPLER_DESC sd;
    sd.AddressU = TranslateTextureAddressMode(desc.wrapModeU);
    sd.AddressV = TranslateTextureAddressMode(desc.wrapModeV);
    sd.AddressW = TranslateTextureAddressMode(desc.wrapModeW);
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

    return true;
}

} // namespace Renderer
} // namespace NFE

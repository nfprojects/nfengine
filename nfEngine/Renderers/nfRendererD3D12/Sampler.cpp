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
    mDesc = desc;
    return true;
}

void Sampler::FillD3DStaticSampler(D3D12_STATIC_SAMPLER_DESC& sampler) const
{
    sampler.AddressU = TranslateTextureAddressMode(mDesc.wrapModeU);
    sampler.AddressV = TranslateTextureAddressMode(mDesc.wrapModeV);
    sampler.AddressW = TranslateTextureAddressMode(mDesc.wrapModeW);
    sampler.MaxAnisotropy = mDesc.maxAnisotropy;
    sampler.Filter = TranslateFilterType(mDesc.minFilter, mDesc.magFilter, mDesc.compare,
                                         mDesc.maxAnisotropy > 1);
    sampler.ComparisonFunc = TranslateComparisonFunc(mDesc.compareFunc);
    sampler.MipLODBias = mDesc.mipmapBias;
    sampler.MinLOD = mDesc.minMipmap;
    sampler.MaxLOD = mDesc.maxMipmap;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
}

} // namespace Renderer
} // namespace NFE

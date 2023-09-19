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

Sampler::Sampler()
    : mDescriptor(UINT32_MAX)
{
}

Sampler::~Sampler()
{
    if (mDescriptor != UINT32_MAX)
    {
        gDevice->GetCbvSrvUavHeapStagingAllocator().Free(mDescriptor, 1);
    }
}

bool Sampler::Init(const SamplerDesc& desc)
{
    NFE_ASSERT(mDescriptor == UINT32_MAX, "Sampler already initialized");

    HeapAllocator& stagingHeapAllocator = gDevice->GetSamplerHeapStagingAllocator();
    mDescriptor = stagingHeapAllocator.Allocate(1);

    if (mDescriptor == UINT32_MAX)
    {
        return false;
    }

    D3D12_SAMPLER_DESC d3dDesc = {};
    d3dDesc.AddressU = TranslateTextureAddressMode(desc.wrapModeU);
    d3dDesc.AddressV = TranslateTextureAddressMode(desc.wrapModeV);
    d3dDesc.AddressW = TranslateTextureAddressMode(desc.wrapModeW);
    d3dDesc.MaxAnisotropy = desc.maxAnisotropy;
    d3dDesc.Filter = TranslateFilterType(desc.minFilter, desc.magFilter, desc.compare, desc.maxAnisotropy > 1);
    d3dDesc.ComparisonFunc = TranslateComparisonFunc(desc.compareFunc);
    d3dDesc.MipLODBias = desc.mipmapBias;
    d3dDesc.MinLOD = desc.minMipmap;
    d3dDesc.MaxLOD = desc.maxMipmap;
    d3dDesc.BorderColor[0] = 0.0f;
    d3dDesc.BorderColor[1] = 0.0f;
    d3dDesc.BorderColor[2] = 0.0f;
    d3dDesc.BorderColor[3] = 0.0f;

    gDevice->GetDevice()->CreateSampler(
        &d3dDesc,
        stagingHeapAllocator.GetCpuHandle(mDescriptor));

    return true;
}

} // namespace Renderer
} // namespace NFE

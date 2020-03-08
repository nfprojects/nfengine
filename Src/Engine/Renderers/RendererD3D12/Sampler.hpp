/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's sampler
 */

#pragma once

#include "../Interface/Texture.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class Sampler : public ISampler
{
    SamplerDesc mDesc;

public:
    bool Init(const SamplerDesc& desc);

    void FillD3DStaticSampler(D3D12_STATIC_SAMPLER_DESC& sampler) const;
};

} // namespace Renderer
} // namespace NFE

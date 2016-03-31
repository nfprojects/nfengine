/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 render's sampler
 */

#pragma once

#include "../RendererInterface/Texture.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class Sampler : public ISampler
{
    friend class CommandBuffer;
    friend class ResourceBindingInstance;

    D3DPtr<ID3D11SamplerState> mSamplerState;
public:
    bool Init(const SamplerDesc& desc);
};

} // namespace Renderer
} // namespace NFE

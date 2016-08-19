/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan renderer's sampler
 */

#pragma once

#include "../RendererInterface/Texture.hpp"
#include "Defines.hpp"

namespace NFE {
namespace Renderer {

class Sampler : public ISampler
{
    friend class CommandBuffer;
    friend class ResourceBindingSet;

    VkSampler mSampler;

public:
    Sampler();
    virtual ~Sampler();
    bool Init(const SamplerDesc& desc);
};

} // namespace Renderer
} // namespace NFE

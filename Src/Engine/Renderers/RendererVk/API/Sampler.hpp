/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan renderer's sampler
 */

#pragma once

#include "../RendererCommon/Texture.hpp"
#include "Defines.hpp"

#include "IResource.hpp"


namespace NFE {
namespace Renderer {

class Sampler : public ISampler, public IResource
{
    friend class CommandRecorder;
    friend class ResourceBindingSet;

    VkSampler mSampler;

public:
    Sampler();
    virtual ~Sampler();
    bool Init(const SamplerDesc& desc);

    const Internal::ResourceType GetType() const override
    {
        return Internal::ResourceType::Sampler;
    }
};

} // namespace Renderer
} // namespace NFE

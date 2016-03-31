/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 renderer's shader resource binding
 */

#pragma once

#include "../RendererInterface/ResourceBinding.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class ResourceBindingSet : public IResourceBindingSet
{
    friend class ResourceBindingInstance;
    friend class CommandBuffer;

    std::vector<ResourceBindingDesc> mBindings;
    ShaderType mShaderVisibility;

public:
    bool Init(const ResourceBindingSetDesc& desc) override;
};

class ResourceBindingLayout : public IResourceBindingLayout
{
    friend class CommandBuffer;

    std::vector<ResourceBindingSet*> mBindingSets;

public:
    bool Init(const ResourceBindingLayoutDesc& desc) override;
};

class ResourceBindingInstance : public IResourceBindingInstance
{
    friend class CommandBuffer;

    ResourceBindingSet* mBindingSet;
    std::vector<void*> mViews; // TODO this can be error-prone

public:
    bool Init(IResourceBindingSet* bindingSet) override;
    bool WriteTexture(size_t slot, ITexture* texture) override;
    bool WriteCBuffer(size_t slot, IBuffer* buffer) override;
    bool WriteSampler(size_t slot, ISampler* sampler);
};

} // namespace Renderer
} // namespace NFE

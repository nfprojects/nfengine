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
    friend class CommandRecorder;

    std::vector<ResourceBindingDesc> mBindings;
    ShaderType mShaderVisibility;

public:
    bool IsBindingOverlapping(const ResourceBindingDesc& bindingDesc) const;
    bool IsBindingSetOverlapping(const ResourceBindingSet* set) const;
    bool Init(const ResourceBindingSetDesc& desc) override;
};

using InternalResourceBindingSetPtr = AtomicSharedPtr<ResourceBindingSet>;


class ResourceBindingLayout : public IResourceBindingLayout
{
    friend class CommandRecorder;

    std::vector<InternalResourceBindingSetPtr> mBindingSets;
    std::vector<VolatileCBufferBinding> mVolatileCBuffers;

public:
    bool Init(const ResourceBindingLayoutDesc& desc) override;
};

class ResourceBindingInstance : public IResourceBindingInstance
{
    friend class CommandRecorder;

    InternalResourceBindingSetPtr mBindingSet;
    std::vector<D3DPtr<ID3D11View>> mViews;
    std::vector<ID3D11Buffer*> mCBuffers;

public:
    bool Init(const ResourceBindingSetPtr& bindingSet) override;
    bool WriteTextureView(size_t slot, const TexturePtr& texture) override;
    bool WriteCBufferView(size_t slot, const BufferPtr& buffer) override;
    bool WriteWritableTextureView(size_t slot, const TexturePtr& texture) override;
};

} // namespace Renderer
} // namespace NFE

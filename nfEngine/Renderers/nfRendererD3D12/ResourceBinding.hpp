/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's shader resource binding
 */

#pragma once

#include "../RendererInterface/ResourceBinding.hpp"
#include "Common.hpp"

#include <vector>

// max supported number of volatile constant buffers
#define NFE_RENDERER_MAX_VOLATILE_CBUFFERS 4

namespace NFE {
namespace Renderer {

class ResourceBindingSet : public IResourceBindingSet
{
    friend class ResourceBindingInstance;
    friend class ResourceBindingLayout;
    friend class CommandRecorder;

    std::vector<ResourceBindingDesc> mBindings;
    ShaderType mShaderVisibility;

public:
    bool Init(const ResourceBindingSetDesc& desc) override;
};

using InternalResourceBindingSetPtr = Common::SharedPtr<ResourceBindingSet>;


//////////////////////////////////////////////////////////////////////////


class ResourceBindingLayout : public IResourceBindingLayout
{
    friend class CommandRecorder;
    friend class PipelineState;

    D3DPtr<ID3D12RootSignature> mRootSignature;
    std::vector<InternalResourceBindingSetPtr> mBindingSets;
    std::vector<VolatileCBufferBinding> mDynamicBuffers;

public:
    bool Init(const ResourceBindingLayoutDesc& desc) override;

    NFE_INLINE ID3D12RootSignature* GetD3DRootSignature() const
    {
        return mRootSignature.Get();
    }
};

using InternalResourceBindingLayoutPtr = Common::SharedPtr<ResourceBindingLayout>;


//////////////////////////////////////////////////////////////////////////


class ResourceBindingInstance : public IResourceBindingInstance
{
    friend class CommandRecorder;

    InternalResourceBindingSetPtr mSet;
    uint32 mDescriptorHeapOffset;

public:
    ResourceBindingInstance() : mDescriptorHeapOffset(0), mSet(nullptr) { }
    ~ResourceBindingInstance();
    bool Init(const ResourceBindingSetPtr& bindingSet) override;
    bool WriteTextureView(size_t slot, const TexturePtr& texture) override;
    bool WriteCBufferView(size_t slot, const BufferPtr& buffer) override;
    bool WriteWritableTextureView(size_t slot, const TexturePtr& texture) override;
};


} // namespace Renderer
} // namespace NFE

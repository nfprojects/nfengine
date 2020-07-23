/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's shader resource binding
 */

#pragma once

#include "../RendererCommon/ResourceBinding.hpp"
#include "Resource.hpp"
#include "../../Common/Containers/DynArray.hpp"


// max supported number of volatile constant buffers
#define NFE_RENDERER_MAX_VOLATILE_CBUFFERS 4

namespace NFE {
namespace Renderer {

class ResourceBindingSet : public IResourceBindingSet
{
    friend class ResourceBindingInstance;
    friend class ResourceBindingLayout;
    friend class CommandRecorder;

    Common::DynArray<ResourceBindingDesc> mBindings;
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
    Common::DynArray<InternalResourceBindingSetPtr> mBindingSets;
    Common::DynArray<VolatileCBufferBinding> mDynamicBuffers;

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

    struct Resource
    {
        // TODO make common class for ITexture and IBuffer
        TexturePtr texture;
        BufferPtr buffer;
    };

    Common::DynArray<Resource> mResources;

public:
    ResourceBindingInstance() : mDescriptorHeapOffset(0), mSet(nullptr) { }
    ~ResourceBindingInstance();
    bool Init(const ResourceBindingSetPtr& bindingSet) override;
    bool WriteTextureView(uint32 slot, const TexturePtr& texture) override;
    bool WriteCBufferView(uint32 slot, const BufferPtr& buffer) override;
    bool WriteWritableTextureView(uint32 slot, const TexturePtr& texture) override;
};


} // namespace Renderer
} // namespace NFE

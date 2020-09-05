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
    // TODO range support
    uint32 GetNumResources() const { return mBindings.Size(); }

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

    uint32 mCpuDescriptorHeapOffset;
    uint32 mGpuDescriptorHeapOffset;
    bool mIsFinalized;

    struct Resource
    {
        // TODO make common class for ITexture and IBuffer
        TexturePtr texture;
        BufferPtr buffer;
    };

    Common::DynArray<Resource> mResources;

public:
    ResourceBindingInstance();
    ~ResourceBindingInstance();

    bool IsFinalized() const { return mIsFinalized; }
    uint32 GetNumResources() const { return mResources.Size(); }

    bool Init(const ResourceBindingSetPtr& bindingSet) override;
    bool SetTextureView(uint32 slot, const TexturePtr& texture, const TextureView& view) override;
    bool SetBufferView(uint32 slot, const BufferPtr& buffer, const BufferView& view) override;
    bool SetCBufferView(uint32 slot, const BufferPtr& buffer) override;
    bool SetWritableTextureView(uint32 slot, const TexturePtr& texture, const TextureView& view) override;
    bool SetWritableBufferView(uint32 slot, const BufferPtr& buffer, const BufferView& view) override;
    bool Finalize() override;
};


//////////////////////////////////////////////////////////////////////////

class Texture;
class Buffer;

void CreateTextureSRV(const Texture* texture, const TextureView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);
void CreateTextureUAV(const Texture* texture, const TextureView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);
void CreateBufferSRV(const Buffer* buffer, const BufferView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);
void CreateBufferUAV(const Buffer* buffer, const BufferView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);

} // namespace Renderer
} // namespace NFE

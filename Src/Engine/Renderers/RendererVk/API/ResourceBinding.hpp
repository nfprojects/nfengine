/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan renderer's shader resource binding
 */

#pragma once

#include "../RendererCommon/ResourceBinding.hpp"
#include "Defines.hpp"
#include "PipelineState.hpp"
#include "IResource.hpp"

#include <Engine/Common/Containers/DynArray.hpp>



namespace NFE {
namespace Renderer {

class ResourceBindingSet : public IResourceBindingSet
{
    friend class ResourceBindingInstance;
    friend class ResourceBindingLayout;
    friend class CommandRecorder;

    VkDescriptorSetLayout mDescriptorLayout;
    uint32 mResourceCount;
    uint32 mTexResourceCount;
    uint16 mSetSlot;

public:
    ResourceBindingSet();
    ~ResourceBindingSet();

    bool Init(const ResourceBindingSetDesc& desc) override;

    NFE_INLINE uint16 GetSetSlot() const
    {
        return mSetSlot;
    }
};

class ResourceBindingLayout : public IResourceBindingLayout
{
    friend class CommandRecorder;
    friend class PipelineState;
    friend class ComputePipelineState;

    VkPipelineLayout mPipelineLayout;
    VkDescriptorSetLayout mVolatileBufferLayout;
    VkDescriptorSet mVolatileBufferSet;
    uint32 mVolatileBufferSetSlot;

public:
    ResourceBindingLayout();
    ~ResourceBindingLayout();

    bool Init(const ResourceBindingLayoutDesc& desc) override;
};

class ResourceBindingInstance : public IResourceBindingInstance
{
    friend class CommandRecorder;

    ResourceBindingSet* mSet;
    VkDescriptorSet mDescriptorSet;
    Common::DynArray<VkImageView> mTextureViews;
    Common::DynArray<IResource*> mWrittenResources; // needed for barriers/image layout transitions
    Common::DynArray<VkDescriptorImageInfo> mImageInfos;
    Common::DynArray<VkDescriptorBufferInfo> mBufferInfos;
    Common::DynArray<VkWriteDescriptorSet> mWrites;

public:
    ~ResourceBindingInstance();

    bool Init(const ResourceBindingSetPtr& bindingSet) override;
    bool SetTextureView(uint32 slot, const TexturePtr& texture, const TextureView& view) override;
    bool SetBufferView(uint32 slot, const BufferPtr& buffer, const BufferView& view) override;
    bool SetCBufferView(uint32 slot, const BufferPtr& buffer) override;
    bool SetWritableTextureView(uint32 slot, const TexturePtr& texture, const TextureView& view) override;
    bool SetWritableBufferView(uint32 slot, const BufferPtr& buffer, const BufferView& view) override;
    bool Finalize() override;
};

} // namespace Renderer
} // namespace NFE

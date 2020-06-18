/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan renderer's shader resource binding
 */

#pragma once

#include "../Interface/ResourceBinding.hpp"
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
    uint32_t mResourceCount;
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
    Common::DynArray<IResource*> mWrittenResources; // needed for barriers/image layout transitions

public:
    bool Init(const ResourceBindingSetPtr& bindingSet) override;
    bool WriteTextureView(uint32 slot, const TexturePtr& texture) override;
    bool WriteCBufferView(uint32 slot, const BufferPtr& buffer) override;
    bool WriteWritableTextureView(uint32 slot, const TexturePtr& texture) override;
};

} // namespace Renderer
} // namespace NFE

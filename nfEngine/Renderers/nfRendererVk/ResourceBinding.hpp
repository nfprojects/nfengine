/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan renderer's shader resource binding
 */

#pragma once

#include "../RendererInterface/ResourceBinding.hpp"
#include "Defines.hpp"
#include "PipelineState.hpp"

#include <vector>
#include <map>


namespace NFE {
namespace Renderer {

class ResourceBindingSet : public IResourceBindingSet
{
    friend class ResourceBindingInstance;
    friend class ResourceBindingLayout;
    friend class CommandBuffer;

    VkDescriptorSet mDescriptorSet;
    VkDescriptorSetLayout mDescriptorLayout;
    uint8 mDescriptorCounter[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
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
    friend class CommandBuffer;
    friend class PipelineState;

    VkPipelineLayout mPipelineLayout;
    VkDescriptorPool mDescriptorPool;
    VkDescriptorSet mVolatileBufferSet;
    VkDescriptorSetLayout mVolatileBufferLayout;
    uint16 mVolatileSetSlot;

public:
    ResourceBindingLayout();
    ~ResourceBindingLayout();

    bool Init(const ResourceBindingLayoutDesc& desc) override;
};

class ResourceBindingInstance : public IResourceBindingInstance
{
    friend class CommandBuffer;

    ResourceBindingSet* mSet;

public:
    bool Init(IResourceBindingSet* bindingSet) override;
    bool WriteTextureView(size_t slot, ITexture* texture) override;
    bool WriteCBufferView(size_t slot, IBuffer* buffer) override;
    bool WriteWritableTextureView(size_t slot, ITexture* texture) override;
};

} // namespace Renderer
} // namespace NFE

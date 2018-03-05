/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan renderer's shader resource binding
 */

#pragma once

#include "../RendererInterface/ResourceBinding.hpp"
#include "Defines.hpp"
#include "PipelineState.hpp"



namespace NFE {
namespace Renderer {

class ResourceBindingSet : public IResourceBindingSet
{
    friend class ResourceBindingInstance;
    friend class ResourceBindingLayout;
    friend class CommandRecorder;

    DevicePtr mDevicePtr;

    VkDescriptorSet mDescriptorSet;
    VkDescriptorSetLayout mDescriptorLayout;
    uint8 mDescriptorCounter[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
    uint16 mSetSlot;

public:
    ResourceBindingSet();
    ~ResourceBindingSet();

    bool Init(DevicePtr& device, const ResourceBindingSetDesc& desc);

    NFE_INLINE uint16 GetSetSlot() const
    {
        return mSetSlot;
    }
};

class ResourceBindingLayout : public IResourceBindingLayout
{
    friend class CommandRecorder;
    friend class PipelineState;

    DevicePtr mDevicePtr;

    VkPipelineLayout mPipelineLayout;
    VkDescriptorPool mDescriptorPool;
    VkDescriptorSet mVolatileBufferSet;
    VkDescriptorSetLayout mVolatileBufferLayout;
    uint16 mVolatileSetSlot;

public:
    ResourceBindingLayout();
    ~ResourceBindingLayout();

    bool Init(DevicePtr& device, const ResourceBindingLayoutDesc& desc);
};

class ResourceBindingInstance : public IResourceBindingInstance
{
    friend class CommandRecorder;

    DevicePtr mDevicePtr;

    // TODO this should be a RBS SharedPtr
    ResourceBindingSet* mSet;

public:
    bool Init(DevicePtr& device, const ResourceBindingSetPtr& bindingSet);
    bool WriteTextureView(uint32 slot, const TexturePtr& texture) override;
    bool WriteCBufferView(uint32 slot, const BufferPtr& buffer) override;
    bool WriteWritableTextureView(uint32 slot, const TexturePtr& texture) override;
};

} // namespace Renderer
} // namespace NFE

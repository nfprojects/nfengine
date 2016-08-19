/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's shader resource binding
 */

#include "PCH.hpp"
#include "ResourceBinding.hpp"
#include "Device.hpp"
#include "Sampler.hpp"


namespace NFE {
namespace Renderer {

bool ResourceBindingSet::Init(const ResourceBindingSetDesc& desc)
{
    mShaderVisibility = desc.shaderVisibility;
    mBindings.reserve(desc.numBindings);
    for (size_t i = 0; i < desc.numBindings; ++i)
        mBindings.push_back(desc.resourceBindings[i]);

    return true;
}


ResourceBindingLayout::ResourceBindingLayout()
    : mPipelineLayout(VK_NULL_HANDLE)
    , mDescriptorLayout(VK_NULL_HANDLE)
    , mDescriptorPool(VK_NULL_HANDLE)
{
}

ResourceBindingLayout::~ResourceBindingLayout()
{
    // there is no need to free separate descriptor sets when pool is freed
    // so, let us not care about these pesky sets
    if (mDescriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(gDevice->GetDevice(), mDescriptorPool, nullptr);
    if (mDescriptorLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(gDevice->GetDevice(), mDescriptorLayout, nullptr);
    if (mPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(gDevice->GetDevice(), mPipelineLayout, nullptr);
}

bool ResourceBindingLayout::Init(const ResourceBindingLayoutDesc& desc)
{
    // TODO support dynamic buffers

    std::vector<VkDescriptorSetLayoutBinding> binding;

    for (size_t i = 0; i < desc.numBindingSets; ++i)
    {
        ResourceBindingSet* rbs = dynamic_cast<ResourceBindingSet*>(desc.bindingSets[i]);
        if (rbs == nullptr)
        {
            LOG_ERROR("Incorrect resource binding set provided at entry %d.", i);
            return false;
        }

        // TODO it might be a good idea to gather resources by type
        VkShaderStageFlags stage = TranslateShaderTypeToVkShaderStageFlags(rbs->mShaderVisibility);
        VkDescriptorSetLayoutBinding layoutBinding;
        layoutBinding.stageFlags = stage;
        for (auto& b: rbs->mBindings)
        {
            layoutBinding.binding = b.slot;
            layoutBinding.descriptorCount = 1;
            layoutBinding.descriptorType = TranslateShaderResourceTypeToVkDescriptorType(b.resourceType);
            if (b.staticSampler)
            {
                Sampler* s = dynamic_cast<Sampler*>(b.staticSampler);
                if (s == nullptr)
                {
                    LOG_ERROR("Incorrect Sampler pointer provided at entry %d.");
                    return false;
                }

                layoutBinding.pImmutableSamplers = &s->mSampler;
            }
            binding.push_back(layoutBinding);
        }
    }

    VkDescriptorSetLayoutCreateInfo descInfo;
    VK_ZERO_MEMORY(descInfo);
    descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descInfo.bindingCount = static_cast<uint32>(binding.size());
    descInfo.pBindings = binding.data();
    VkResult result = vkCreateDescriptorSetLayout(gDevice->GetDevice(), &descInfo, nullptr, &mDescriptorLayout);
    CHECK_VKRESULT(result, "Failed to create Descriptor Set Layout");

    VkPipelineLayoutCreateInfo plInfo;
    VK_ZERO_MEMORY(plInfo);
    plInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plInfo.setLayoutCount = 1;
    plInfo.pSetLayouts = &mDescriptorLayout;
    plInfo.pushConstantRangeCount = 0;
    result = vkCreatePipelineLayout(gDevice->GetDevice(), &plInfo, nullptr, &mPipelineLayout);
    CHECK_VKRESULT(result, "Failed to create pipeline layout");

    // layouts created, now it's time to allocate the pool and our descriptors
    // first - the pool
    VkDescriptorPoolCreateInfo poolInfo;
    VK_ZERO_MEMORY(poolInfo);


    return true;
}


bool ResourceBindingInstance::Init(IResourceBindingSet* bindingSet)
{
    mSet = dynamic_cast<ResourceBindingSet*>(bindingSet);
    if (mSet == nullptr)
    {
        LOG_ERROR("Incorrect binding set provided");
        return false;
    }

    return true;
}

bool ResourceBindingInstance::WriteTextureView(size_t slot, ITexture* texture)
{
    // vkUpdateDescriptorSets
    // VkDescriptorImageInfo
    UNUSED(slot);
    UNUSED(texture);
    return false;
}

bool ResourceBindingInstance::WriteCBufferView(size_t slot, IBuffer* buffer)
{
    // vkUpdateDescriptorSets
    // VkDescriptorBufferInfo
    UNUSED(slot);
    UNUSED(buffer);
    return false;
}

} // namespace Renderer
} // namespace NFE

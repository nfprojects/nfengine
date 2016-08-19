/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's shader resource binding
 */

#include "PCH.hpp"
#include "ResourceBinding.hpp"
#include "Device.hpp"
#include "Sampler.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"


namespace NFE {
namespace Renderer {

ResourceBindingSet::ResourceBindingSet()
    : mDescriptorSet(VK_NULL_HANDLE)
    , mDescriptorLayout(VK_NULL_HANDLE)
{
}

ResourceBindingSet::~ResourceBindingSet()
{
    // we leave descriptor sets alone - they will be freed together with their descriptor pool
    if (mDescriptorLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(gDevice->GetDevice(), mDescriptorLayout, nullptr);
}

bool ResourceBindingSet::Init(const ResourceBindingSetDesc& desc)
{
    // each set has its own layout
    // gather the layout right here, will be useful to allocate proper sets later on
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    VkShaderStageFlags stage = TranslateShaderTypeToVkShaderStageFlags(desc.shaderVisibility);
    VkDescriptorSetLayoutBinding layoutBinding;
    layoutBinding.stageFlags = stage;
    for (uint32 i = 0; i < desc.numBindings; ++i)
    {
        ResourceBindingDesc& rb = desc.resourceBindings[i];
        layoutBinding.binding = rb.slot;
        layoutBinding.descriptorCount = 1;
        layoutBinding.descriptorType = TranslateShaderResourceTypeToVkDescriptorType(rb.resourceType);
        if (rb.staticSampler)
        {
            Sampler* s = dynamic_cast<Sampler*>(rb.staticSampler);
            if (s == nullptr)
            {
                LOG_ERROR("Incorrect Sampler pointer provided at entry %d.");
                return false;
            }

            layoutBinding.pImmutableSamplers = &s->mSampler;
        }
        bindings.push_back(layoutBinding);

        mDescriptorCounter[layoutBinding.descriptorType]++;
    }

    VkDescriptorSetLayoutCreateInfo descInfo;
    VK_ZERO_MEMORY(descInfo);
    descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descInfo.bindingCount = static_cast<uint32>(bindings.size());
    descInfo.pBindings = bindings.data();
    VkResult result = vkCreateDescriptorSetLayout(gDevice->GetDevice(), &descInfo, nullptr, &mDescriptorLayout);
    CHECK_VKRESULT(result, "Failed to create Descriptor Set Layout");

    return true;
}


ResourceBindingLayout::ResourceBindingLayout()
    : mDynamicBufferSet(VK_NULL_HANDLE)
    , mDynamicBufferLayout(VK_NULL_HANDLE)
    , mPipelineLayout(VK_NULL_HANDLE)
    , mDescriptorPool(VK_NULL_HANDLE)
{
}

ResourceBindingLayout::~ResourceBindingLayout()
{
    // there is no need to free separate descriptor sets when pool is freed
    // we can just free the pool and sets will follow with it
    if (mDescriptorPool != VK_NULL_HANDLE)
    {
        vkResetDescriptorPool(gDevice->GetDevice(), mDescriptorPool, 0);
        vkDestroyDescriptorPool(gDevice->GetDevice(), mDescriptorPool, nullptr);
    }
    if (mPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(gDevice->GetDevice(), mPipelineLayout, nullptr);
    if (mDynamicBufferLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(gDevice->GetDevice(), mDynamicBufferLayout, nullptr);
}

bool ResourceBindingLayout::Init(const ResourceBindingLayoutDesc& desc)
{
    // TODO support dynamic buffers
    std::map<VkDescriptorType, uint32> descSizes;
    std::vector<VkDescriptorSetLayoutBinding> dynBufferBindings;
    std::vector<VkDescriptorSetLayout> layouts;

    for (uint32 i = 0; i < desc.numBindingSets; ++i)
    {
        ResourceBindingSet* rbs = dynamic_cast<ResourceBindingSet*>(desc.bindingSets[i]);

        // summarize our descriptor sizes (needed by descriptor pool)
        for (auto& d: rbs->mDescriptorCounter)
            descSizes[d.first] += d.second;

        layouts.push_back(rbs->mDescriptorLayout);
    }

    // create additional layout to support dynamic buffers
    for (uint32 buf = 0; buf < desc.numDynamicBuffers; ++buf)
    {
        const DynamicBufferBindingDesc& dbDesc = desc.dynamicBuffers[buf];

        VkDescriptorSetLayoutBinding dslBinding;
        VK_ZERO_MEMORY(dslBinding);
        dslBinding.stageFlags = TranslateShaderTypeToVkShaderStageFlags(dbDesc.shaderVisibility);
        dslBinding.binding = dbDesc.slot;
        dslBinding.descriptorCount = 1;
        dslBinding.descriptorType = TranslateDynamicResourceTypeToVkDescriptorType(dbDesc.resourceType);
        if (dslBinding.descriptorType == VK_DESCRIPTOR_TYPE_MAX_ENUM)
        {
            LOG_ERROR("Unsupported dynamic resource type provided");
            return false;
        }

        dynBufferBindings.push_back(dslBinding);
        descSizes[dslBinding.descriptorType]++;
    }

    VkDescriptorSetLayoutCreateInfo dslInfo;
    VK_ZERO_MEMORY(dslInfo);
    dslInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dslInfo.bindingCount = static_cast<uint32>(dynBufferBindings.size());
    dslInfo.pBindings = dynBufferBindings.data();
    VkResult result = vkCreateDescriptorSetLayout(gDevice->GetDevice(), &dslInfo, nullptr, &mDynamicBufferLayout);
    CHECK_VKRESULT(result, "Failed to create layout for dynamic buffers");
    layouts.push_back(mDynamicBufferLayout);


    // join all layouts into a Pipeline Layout
    VkPipelineLayoutCreateInfo plInfo;
    VK_ZERO_MEMORY(plInfo);
    plInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plInfo.setLayoutCount = static_cast<uint32>(desc.numBindingSets);
    plInfo.pSetLayouts = layouts.data();
    plInfo.pushConstantRangeCount = 0;
    result = vkCreatePipelineLayout(gDevice->GetDevice(), &plInfo, nullptr, &mPipelineLayout);
    CHECK_VKRESULT(result, "Failed to create pipeline layout");

    // TODO this might be a bad idea to allocate sets this way - consider other solutions
    // generate pool sizes from descriptor sets
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (auto& d: descSizes)
    {
        VkDescriptorPoolSize size;
        size.type = d.first;
        size.descriptorCount = d.second;
        poolSizes.push_back(size);
    }

    // perform set allocation, but only when there is something to allocate
    if (desc.numBindingSets + desc.numDynamicBuffers > 0)
    {
        // allocate pool for descriptors
        VkDescriptorPoolCreateInfo poolInfo;
        VK_ZERO_MEMORY(poolInfo);
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = static_cast<uint32>(desc.numBindingSets + 1);
        poolInfo.poolSizeCount = static_cast<uint32>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        result = vkCreateDescriptorPool(gDevice->GetDevice(), &poolInfo, nullptr, &mDescriptorPool);
        CHECK_VKRESULT(result, "Failed to create descriptor pool");

        // now allocate sets from pool
        // TODO it would be faster to allocate all sets at once
        VkDescriptorSetAllocateInfo allocInfo;
        VK_ZERO_MEMORY(allocInfo);
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDescriptorPool;

        for (size_t i = 0; i < desc.numBindingSets; ++i)
        {
            ResourceBindingSet* rbs = dynamic_cast<ResourceBindingSet*>(desc.bindingSets[i]);
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &layouts[i];
            vkAllocateDescriptorSets(gDevice->GetDevice(), &allocInfo, &rbs->mDescriptorSet);
        }

        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &mDynamicBufferLayout;
        vkAllocateDescriptorSets(gDevice->GetDevice(), &allocInfo, &mDynamicBufferSet);
    }

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
    Texture* t = dynamic_cast<Texture*>(texture);
    if (t == nullptr)
    {
        LOG_ERROR("Incorrect Texture pointer provided");
        return false;
    }

    VkDescriptorImageInfo imgInfo;
    VK_ZERO_MEMORY(imgInfo);
    imgInfo.imageView = 0;//t->mImageView;

    VkWriteDescriptorSet writeSet;
    VK_ZERO_MEMORY(writeSet);
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = mSet->mDescriptorSet;
    writeSet.dstBinding = static_cast<uint32>(slot);
    writeSet.dstArrayElement = 0;
    writeSet.descriptorCount = 1;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writeSet.pImageInfo = &imgInfo;

    // TODO uncomment when textures are supported properly
    //vkUpdateDescriptorSets(gDevice->GetDevice(), 1, &writeSet, 0, nullptr);
    return false;
}

bool ResourceBindingInstance::WriteCBufferView(size_t slot, IBuffer* buffer)
{

    Buffer* b = dynamic_cast<Buffer*>(buffer);
    if (b == nullptr)
    {
        LOG_ERROR("Incorrect Buffer pointer provided");
        return false;
    }

    VkDescriptorBufferInfo bufInfo;
    VK_ZERO_MEMORY(bufInfo);
    bufInfo.buffer = b->mBuffer;
    bufInfo.offset = 0;
    bufInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet writeSet;
    VK_ZERO_MEMORY(writeSet);
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = mSet->mDescriptorSet;
    writeSet.dstBinding = static_cast<uint32>(slot);
    writeSet.dstArrayElement = 0;
    writeSet.descriptorCount = 1;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeSet.pBufferInfo = &bufInfo;

    vkUpdateDescriptorSets(gDevice->GetDevice(), 1, &writeSet, 0, nullptr);

    return true;
}

} // namespace Renderer
} // namespace NFE

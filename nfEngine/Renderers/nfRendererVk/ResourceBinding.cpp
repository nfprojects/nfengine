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
    for (int i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i)
        mDescriptorCounter[i] = 0;
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

    VkShaderStageFlags stage = TranslateShaderTypeToVkShaderStage(desc.shaderVisibility);
    VkDescriptorSetLayoutBinding layoutBinding;
    VK_ZERO_MEMORY(layoutBinding);
    layoutBinding.stageFlags = stage;
    for (uint32 i = 0; i < desc.numBindings; ++i)
    {
        ResourceBindingDesc& rb = desc.resourceBindings[i];
        layoutBinding.binding = rb.slot;
        layoutBinding.descriptorCount = 1;
        layoutBinding.descriptorType = TranslateShaderResourceTypeToVkDescriptorType(rb.resourceType);
        if (rb.staticSampler && (layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
                                 layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER))
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
    : mPipelineLayout(VK_NULL_HANDLE)
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
}

bool ResourceBindingLayout::Init(const ResourceBindingLayoutDesc& desc)
{
    VkResult result = VK_SUCCESS;
    uint8 descSizes[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
    std::vector<VkDescriptorSetLayoutBinding> volatileBindings;
    std::vector<VkDescriptorSetLayout> layouts;

    if (desc.numVolatileCBuffers > 0)
    {
        LOG_ERROR("Volatile CBuffers are unsupported.");
        return false;
    }

    for (int d = 0; d < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++d)
        descSizes[d] = 0;

    for (uint32 i = 0; i < desc.numBindingSets; ++i)
    {
        ResourceBindingSet* rbs = dynamic_cast<ResourceBindingSet*>(desc.bindingSets[i]);

        // summarize our descriptor sizes (needed by descriptor pool)
        for (int d = 0; d < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++d)
            if (rbs->mDescriptorCounter[d])
                descSizes[d] += rbs->mDescriptorCounter[d];

        layouts.push_back(rbs->mDescriptorLayout);
    }

    // join all layouts into a Pipeline Layout
    VkPipelineLayoutCreateInfo plInfo;
    VK_ZERO_MEMORY(plInfo);
    plInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plInfo.setLayoutCount = static_cast<uint32>(layouts.size());
    plInfo.pSetLayouts = layouts.data();
    result = vkCreatePipelineLayout(gDevice->GetDevice(), &plInfo, nullptr, &mPipelineLayout);
    CHECK_VKRESULT(result, "Failed to create pipeline layout");

    // TODO this might be a bad idea to allocate sets this way - consider other solutions
    // generate pool sizes from descriptor sets
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (int d = 0; d < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++d)
    {
        if (descSizes[d])
        {
            VkDescriptorPoolSize size;
            size.type = static_cast<VkDescriptorType>(d);
            size.descriptorCount = descSizes[d];
            poolSizes.push_back(size);
        }
    }

    // perform set allocation, but only when there is something to allocate
    if (desc.numBindingSets > 0)
    {
        // allocate pool for descriptors
        VkDescriptorPoolCreateInfo poolInfo;
        VK_ZERO_MEMORY(poolInfo);
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = static_cast<uint32>(desc.numBindingSets);
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
            if (rbs == nullptr)
            {
                LOG_ERROR("Incorrect resource binding set provided at index %i", i);
                continue;
            }

            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &layouts[i];
            result = vkAllocateDescriptorSets(gDevice->GetDevice(), &allocInfo, &(rbs->mDescriptorSet));
            CHECK_VKRESULT(result, "Failed to allocate descriptor set from pool");
        }
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
    bufInfo.range = b->mBufferSize;

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

bool ResourceBindingInstance::WriteWritableTextureView(size_t slot, ITexture* texture)
{
    UNUSED(slot);
    UNUSED(texture);
    return false;
}

} // namespace Renderer
} // namespace NFE

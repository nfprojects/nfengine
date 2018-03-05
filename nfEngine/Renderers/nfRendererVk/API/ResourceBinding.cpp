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
    Common::DynArray<VkDescriptorSetLayoutBinding> bindings;

    VkShaderStageFlags stage = TranslateShaderTypeToVkShaderStage(desc.shaderVisibility);
    VkDescriptorSetLayoutBinding layoutBinding;
    VK_ZERO_MEMORY(layoutBinding);
    layoutBinding.stageFlags = stage;

    bool setApplied = false;
    SetBindingPair bindingSlot;
    for (uint32 i = 0; i < desc.numBindings; ++i)
    {
        ResourceBindingDesc rb = desc.resourceBindings[i];
        bindingSlot.total = rb.slot;

        // decode set and bind from slot value
        if (setApplied)
        {
            if (bindingSlot.pair.set != mSetSlot)
            {
                NFE_LOG_ERROR("Supplied bindings do not belong in the same set");
                return false;
            }
        }
        else
        {
            mSetSlot = bindingSlot.pair.set;
            setApplied = true;
        }

        layoutBinding.binding = bindingSlot.pair.binding;
        layoutBinding.descriptorCount = 1;
        layoutBinding.descriptorType = TranslateShaderResourceTypeToVkDescriptorType(rb.resourceType);
        if (rb.staticSampler)
        {
            Sampler* s = dynamic_cast<Sampler*>(rb.staticSampler.Get());
            if (s == nullptr)
            {
                NFE_LOG_ERROR("Incorrect Sampler pointer provided at entry %d.");
                return false;
            }

            layoutBinding.pImmutableSamplers = &s->mSampler;
        }
        bindings.PushBack(layoutBinding);

        mDescriptorCounter[layoutBinding.descriptorType]++;
    }

    VkDescriptorSetLayoutCreateInfo descInfo;
    VK_ZERO_MEMORY(descInfo);
    descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descInfo.bindingCount = bindings.Size();
    descInfo.pBindings = bindings.Data();
    VkResult result = vkCreateDescriptorSetLayout(gDevice->GetDevice(), &descInfo, nullptr, &mDescriptorLayout);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to create Descriptor Set Layout");

    return true;
}


ResourceBindingLayout::ResourceBindingLayout()
    : mPipelineLayout(VK_NULL_HANDLE)
    , mDescriptorPool(VK_NULL_HANDLE)
    , mVolatileBufferSet(VK_NULL_HANDLE)
    , mVolatileBufferLayout(VK_NULL_HANDLE)
    , mVolatileSetSlot(UINT16_MAX)
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
    if (mVolatileBufferLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(gDevice->GetDevice(), mVolatileBufferLayout, nullptr);

    mVolatileSetSlot = UINT16_MAX;
}

bool ResourceBindingLayout::Init(const ResourceBindingLayoutDesc& desc)
{
    VkResult result = VK_SUCCESS;
    uint8 descSizes[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
    Common::DynArray<VkDescriptorSetLayoutBinding> volatileBindings;
    Common::DynArray<VkDescriptorSetLayout> layouts;

    for (int d = 0; d < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++d)
        descSizes[d] = 0;

    // create additional layout to support dynamic buffers
    SetBindingPair bindingSlot;
    bool volatileSetApplied = false;
    for (uint32 buf = 0; buf < desc.numVolatileCBuffers; ++buf)
    {
        const VolatileCBufferBinding& vb = desc.volatileCBuffers[buf];
        bindingSlot.total = vb.slot;

        VkDescriptorSetLayoutBinding dslBinding;
        VK_ZERO_MEMORY(dslBinding);
        dslBinding.stageFlags = TranslateShaderTypeToVkShaderStage(vb.shaderVisibility);

        // decode set and bind from slot value
        if (volatileSetApplied)
        {
            if (bindingSlot.pair.set != mVolatileSetSlot)
            {
                NFE_LOG_ERROR("Supplied volatile bindings do not belong in the same set");
                return false;
            }
        }
        else
        {
            mVolatileSetSlot = bindingSlot.pair.set;
            volatileSetApplied = true;
        }

        dslBinding.binding = bindingSlot.pair.binding;
        dslBinding.descriptorCount = 1;
        dslBinding.descriptorType = TranslateDynamicResourceTypeToVkDescriptorType(vb.resourceType);
        if (dslBinding.descriptorType == VK_DESCRIPTOR_TYPE_MAX_ENUM)
        {
            NFE_LOG_ERROR("Unsupported dynamic resource type provided");
            return false;
        }

        volatileBindings.PushBack(dslBinding);
        descSizes[dslBinding.descriptorType]++;
    }

    if (!volatileBindings.Empty())
    {
        VkDescriptorSetLayoutCreateInfo dslInfo;
        VK_ZERO_MEMORY(dslInfo);
        dslInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dslInfo.bindingCount = volatileBindings.Size();
        dslInfo.pBindings = volatileBindings.Data();
        result = vkCreateDescriptorSetLayout(gDevice->GetDevice(), &dslInfo, nullptr, &mVolatileBufferLayout);
        VK_RETURN_FALSE_IF_FAILED(result, "Failed to create layout for volatile buffers");
    }

    // sort binding sets according to their internal mSetSlot variable
    Common::DynArray<ResourceBindingSet*> setPtrs;
    setPtrs.Resize(desc.numBindingSets);
    for (uint32 i = 0; i < desc.numBindingSets; ++i)
        setPtrs[i] = dynamic_cast<ResourceBindingSet*>(desc.bindingSets[i].Get());

    std::sort(setPtrs.begin(), setPtrs.end(), [](ResourceBindingSet* rbs1, ResourceBindingSet* rbs2) {
        return rbs1->GetSetSlot() < rbs2->GetSetSlot();
    });

    bool volatileLayoutAdded = false;
    for (auto rbs : setPtrs)
    {
        // summarize our descriptor sizes (needed by descriptor pool)
        for (int d = 0; d < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++d)
            if (rbs->mDescriptorCounter[d])
                descSizes[d] += rbs->mDescriptorCounter[d];

        if (rbs->mSetSlot == mVolatileSetSlot)
        {
            NFE_LOG_ERROR("A volatile buffer has the same set slot assigned as one of the binding sets.");
            return false;
        }

        if (rbs->mSetSlot > mVolatileSetSlot)
        {
            if (!volatileLayoutAdded)
            {
                layouts.PushBack(mVolatileBufferLayout);
                volatileLayoutAdded = true;
            }
        }

        layouts.PushBack(rbs->mDescriptorLayout);
    }

    if (layouts.Empty())
        if (desc.numVolatileCBuffers > 0)
            layouts.PushBack(mVolatileBufferLayout);

    // join all layouts into a Pipeline Layout
    VkPipelineLayoutCreateInfo plInfo;
    VK_ZERO_MEMORY(plInfo);
    plInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plInfo.setLayoutCount = layouts.Size();
    plInfo.pSetLayouts = layouts.Data();
    result = vkCreatePipelineLayout(gDevice->GetDevice(), &plInfo, nullptr, &mPipelineLayout);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to create pipeline layout");

    // TODO this might be a bad idea to allocate sets this way - consider other solutions
    // generate pool sizes from descriptor sets
    Common::StaticArray<VkDescriptorPoolSize, VK_DESCRIPTOR_TYPE_RANGE_SIZE> poolSizes;
    for (int d = 0; d < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++d)
    {
        if (descSizes[d])
        {
            VkDescriptorPoolSize size;
            size.type = static_cast<VkDescriptorType>(d);
            size.descriptorCount = descSizes[d];
            poolSizes.PushBack(size);
        }
    }

    // perform set allocation, but only when there is something to allocate
    if (desc.numBindingSets + desc.numVolatileCBuffers > 0)
    {
        // allocate pool for descriptors
        VkDescriptorPoolCreateInfo poolInfo;
        VK_ZERO_MEMORY(poolInfo);
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = desc.numBindingSets;
        if (desc.numVolatileCBuffers)
            poolInfo.maxSets++; // plus one Volatile Buffer set
        poolInfo.poolSizeCount = poolSizes.Size();
        poolInfo.pPoolSizes = poolSizes.Data();
        result = vkCreateDescriptorPool(gDevice->GetDevice(), &poolInfo, nullptr, &mDescriptorPool);
        VK_RETURN_FALSE_IF_FAILED(result, "Failed to create descriptor pool");

        // now allocate sets from pool
        // TODO it would be faster to allocate all sets at once
        VkDescriptorSetAllocateInfo allocInfo;
        VK_ZERO_MEMORY(allocInfo);
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDescriptorPool;

        for (auto rbs : setPtrs)
        {
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &rbs->mDescriptorLayout;
            result = vkAllocateDescriptorSets(gDevice->GetDevice(), &allocInfo, &(rbs->mDescriptorSet));
            VK_RETURN_FALSE_IF_FAILED(result, "Failed to allocate descriptor set from pool");
        }

        if (desc.numVolatileCBuffers > 0)
        {
            allocInfo.descriptorSetCount = static_cast<uint32>(desc.numVolatileCBuffers);
            allocInfo.pSetLayouts = &mVolatileBufferLayout;
            result = vkAllocateDescriptorSets(gDevice->GetDevice(), &allocInfo, &mVolatileBufferSet);
            VK_RETURN_FALSE_IF_FAILED(result, "Failed to allocate descriptor set from pool");
        }
    }

    if (desc.numVolatileCBuffers > 0)
    {
        VkDescriptorBufferInfo bufInfo;
        VK_ZERO_MEMORY(bufInfo);
        bufInfo.buffer = gDevice->GetRingBuffer()->GetVkBuffer();
        bufInfo.offset = 0;
        bufInfo.range = desc.volatileCBuffers[0].size; // FIXME we must know buffer's size here

        VkWriteDescriptorSet writeSet;
        VK_ZERO_MEMORY(writeSet);
        writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSet.dstSet = mVolatileBufferSet;
        writeSet.dstBinding = 0;
        writeSet.dstArrayElement = 0;
        writeSet.descriptorCount = 1;
        writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writeSet.pBufferInfo = &bufInfo;

        vkUpdateDescriptorSets(gDevice->GetDevice(), 1, &writeSet, 0, nullptr);
    }

    return true;
}


bool ResourceBindingInstance::Init(const ResourceBindingSetPtr& bindingSet)
{
    mSet = dynamic_cast<ResourceBindingSet*>(bindingSet.Get());
    if (mSet == nullptr)
    {
        NFE_LOG_ERROR("Incorrect binding set provided");
        return false;
    }

    return true;
}

bool ResourceBindingInstance::WriteTextureView(uint32 slot, const TexturePtr& texture)
{
    Texture* t = dynamic_cast<Texture*>(texture.Get());
    if (t == nullptr)
    {
        NFE_LOG_ERROR("Incorrect Texture pointer provided");
        return false;
    }

    VkDescriptorImageInfo imgInfo;
    VK_ZERO_MEMORY(imgInfo);
    imgInfo.imageView = t->mImages[t->mCurrentBuffer].view;

    VkWriteDescriptorSet writeSet;
    VK_ZERO_MEMORY(writeSet);
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = mSet->mDescriptorSet;
    writeSet.dstBinding = slot;
    writeSet.dstArrayElement = 0;
    writeSet.descriptorCount = 1;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeSet.pImageInfo = &imgInfo;

    vkUpdateDescriptorSets(gDevice->GetDevice(), 1, &writeSet, 0, nullptr);

    return true;
}

bool ResourceBindingInstance::WriteCBufferView(uint32 slot, const BufferPtr& buffer)
{
    Buffer* b = dynamic_cast<Buffer*>(buffer.Get());
    if (b == nullptr)
    {
        NFE_LOG_ERROR("Incorrect Buffer pointer provided");
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
    writeSet.dstBinding = slot;
    writeSet.dstArrayElement = 0;
    writeSet.descriptorCount = 1;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeSet.pBufferInfo = &bufInfo;

    vkUpdateDescriptorSets(gDevice->GetDevice(), 1, &writeSet, 0, nullptr);

    return true;
}

bool ResourceBindingInstance::WriteWritableTextureView(uint32 slot, const TexturePtr& texture)
{
    NFE_UNUSED(slot);
    NFE_UNUSED(texture);
    return false;
}

} // namespace Renderer
} // namespace NFE
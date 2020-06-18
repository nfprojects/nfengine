/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's shader resource binding
 */

#include "PCH.hpp"
#include "ResourceBinding.hpp"
#include "Device.hpp"
#include "Sampler.hpp"

#include "Engine/Common/Containers/StaticArray.hpp"
#include "Internal/Debugger.hpp"


namespace NFE {
namespace Renderer {

ResourceBindingSet::ResourceBindingSet()
    : mDescriptorLayout(VK_NULL_HANDLE)
    , mSetSlot(UINT16_MAX)
{
}

ResourceBindingSet::~ResourceBindingSet()
{
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
    for (uint32 i = 0; i < desc.numBindings; ++i)
    {
        ResourceBindingDesc rb = desc.resourceBindings[i];

        // decode set and bind from slot value
        if (setApplied)
        {
            if ((rb.slot >> 16) != mSetSlot)
            {
                NFE_LOG_ERROR("Supplied bindings do not belong in the same descriptor set.");
                return false;
            }
        }
        else
        {
            mSetSlot = rb.slot >> 16;
            setApplied = true;
        }

        layoutBinding.binding = rb.slot & 0xFFFF;
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
    }

    VkDescriptorSetLayoutCreateInfo descInfo;
    VK_ZERO_MEMORY(descInfo);
    descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descInfo.bindingCount = bindings.Size();
    descInfo.pBindings = bindings.Data();
    VkResult result = vkCreateDescriptorSetLayout(gDevice->GetDevice(), &descInfo, nullptr, &mDescriptorLayout);
    CHECK_VKRESULT(result, "Failed to create Descriptor Set Layout");

    Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mDescriptorLayout), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "DescriptorSetLayout");

    return true;
}


ResourceBindingLayout::ResourceBindingLayout()
    : mPipelineLayout(VK_NULL_HANDLE)
    , mVolatileBufferLayout(VK_NULL_HANDLE)
    , mVolatileBufferSet(VK_NULL_HANDLE)
    , mVolatileBufferSetSlot(UINT32_MAX)
{
}

ResourceBindingLayout::~ResourceBindingLayout()
{
    if (mVolatileBufferSet != VK_NULL_HANDLE)
        vkFreeDescriptorSets(gDevice->GetDevice(), gDevice->GetDescriptorPool(), 1, &mVolatileBufferSet);
    if (mVolatileBufferLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(gDevice->GetDevice(), mVolatileBufferLayout, nullptr);
    if (mPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(gDevice->GetDevice(), mPipelineLayout, nullptr);
}

bool ResourceBindingLayout::Init(const ResourceBindingLayoutDesc& desc)
{
    VkResult result = VK_SUCCESS;

    Common::DynArray<VkDescriptorSetLayout> dsls;
    const uint32 totalSetCount = desc.numBindingSets + desc.numVolatileCBuffers;
    dsls.Resize(totalSetCount);
    for (uint32 i = 0; i < desc.numBindingSets; ++i)
    {
        ResourceBindingSet* rbs = dynamic_cast<ResourceBindingSet*>(desc.bindingSets[i].Get());
        if (rbs == nullptr)
        {
            NFE_LOG_ERROR("Invalid Resource Binding Set provided at %d spot", i);
            return false;
        }

        if (rbs->mSetSlot > totalSetCount)
        {
            NFE_LOG_ERROR("Resource Binding Layout has too high set slot assigned.");
            return false;
        }

        dsls[rbs->mSetSlot] = rbs->mDescriptorLayout;
    }

    if (desc.numVolatileCBuffers > 0)
    {
        Common::DynArray<VkDescriptorSetLayoutBinding> volatileCBufferBindings;
        bool setApplied = false;
        VkDescriptorSetLayoutBinding layoutBinding;
        VK_ZERO_MEMORY(layoutBinding);
        for (uint32 i = 0; i < desc.numVolatileCBuffers; ++i)
        {
            const VolatileCBufferBinding& vb = desc.volatileCBuffers[i];

            // decode set and bind from slot value
            if (setApplied)
            {
                if ((vb.slot >> 16) != mVolatileBufferSetSlot)
                {
                    NFE_LOG_ERROR("Supplied bindings do not belong in the same descriptor set.");
                    return false;
                }
            }
            else
            {
                mVolatileBufferSetSlot = vb.slot >> 16;
                setApplied = true;
            }

            layoutBinding.stageFlags = TranslateShaderTypeToVkShaderStage(vb.shaderVisibility);
            layoutBinding.binding = vb.slot;
            layoutBinding.descriptorCount = 1;
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            layoutBinding.pImmutableSamplers = nullptr;
            volatileCBufferBindings.PushBack(layoutBinding);
        }

        if (mVolatileBufferSetSlot > totalSetCount)
        {
            NFE_LOG_ERROR("Resource Binding Layout has too high set slot assigned.");
            return false;
        }

        VkDescriptorSetLayoutCreateInfo descInfo;
        VK_ZERO_MEMORY(descInfo);
        descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descInfo.bindingCount = volatileCBufferBindings.Size();
        descInfo.pBindings = volatileCBufferBindings.Data();
        result = vkCreateDescriptorSetLayout(gDevice->GetDevice(), &descInfo, nullptr, &mVolatileBufferLayout);
        CHECK_VKRESULT(result, "Failed to create Volatile CBuffer Descriptor Set Layout");

        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mVolatileBufferLayout), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "VolatileBufferDSL");

        dsls[mVolatileBufferSetSlot] = mVolatileBufferLayout;
    }

    VkPipelineLayoutCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pSetLayouts = dsls.Data();
    info.setLayoutCount = dsls.Size();
    result = vkCreatePipelineLayout(gDevice->GetDevice(), &info, nullptr, &mPipelineLayout);
    CHECK_VKRESULT(result, "Failed to create Pipeline Layout for Resource Binding Layout");

    if (desc.debugName)
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mPipelineLayout), VK_OBJECT_TYPE_PIPELINE_LAYOUT, desc.debugName);

    if (desc.numVolatileCBuffers > 0)
    {
        VkDescriptorSetAllocateInfo allocInfo;
        VK_ZERO_MEMORY(allocInfo);
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = gDevice->GetDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &mVolatileBufferLayout;
        result = vkAllocateDescriptorSets(gDevice->GetDevice(), &allocInfo, &mVolatileBufferSet);
        CHECK_VKRESULT(result, "Failed to allocate Volatile Buffer Set");

        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mVolatileBufferSet), VK_OBJECT_TYPE_DESCRIPTOR_SET, "VolatileBufferSet");

        for (uint32 i = 0; i < desc.numVolatileCBuffers; ++i)
        {
            const VolatileCBufferBinding& vb = desc.volatileCBuffers[i];

            // NOTE I am not sure how Vulkan will behave if we bind the same buffer
            //      to different descriptors. Check that later on.
            VkDescriptorBufferInfo bufInfo;
            VK_ZERO_MEMORY(bufInfo);
            bufInfo.buffer = gDevice->GetRingBuffer()->GetVkBuffer();
            bufInfo.offset = 0;
            bufInfo.range = vb.size;

            VkWriteDescriptorSet writeSet;
            VK_ZERO_MEMORY(writeSet);
            writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeSet.dstSet = mVolatileBufferSet;
            writeSet.dstBinding = (vb.slot & 0xFFFF);
            writeSet.dstArrayElement = 0;
            writeSet.descriptorCount = 1;
            writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            writeSet.pBufferInfo = &bufInfo;

            vkUpdateDescriptorSets(gDevice->GetDevice(), 1, &writeSet, 0, nullptr);
        }
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

    VkResult result = VK_SUCCESS;
    VkDescriptorSetAllocateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool = gDevice->GetDescriptorPool();
    info.descriptorSetCount = 1;
    info.pSetLayouts = &mSet->mDescriptorLayout;
    result = vkAllocateDescriptorSets(gDevice->GetDevice(), &info, &mDescriptorSet);
    CHECK_VKRESULT(result, "Failed to allocate Descriptor Set for Resource Binding Instance");

    Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mDescriptorSet), VK_OBJECT_TYPE_DESCRIPTOR_SET, "DescriptorSet");

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
    imgInfo.imageView = t->mImageView;
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writeSet;
    VK_ZERO_MEMORY(writeSet);
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = mDescriptorSet;
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
    writeSet.dstSet = mDescriptorSet;
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

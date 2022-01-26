/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's shader resource binding
 */

#include "PCH.hpp"
#include "ResourceBinding.hpp"
#include "Device.hpp"
#include "Sampler.hpp"

#include "Engine/Common/Containers/StaticArray.hpp"
#include "Internal/Debugger.hpp"
#include "Internal/Utilities.hpp"


namespace NFE {
namespace Renderer {

ResourceBindingSet::ResourceBindingSet()
    : mDescriptorLayout(VK_NULL_HANDLE)
    , mShaderStage(0)
    , mResourceCount(0)
    , mTexResourceCount(0)
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

    mShaderStage = TranslateShaderTypeToVkShaderStage(desc.shaderVisibility);
    VkDescriptorSetLayoutBinding layoutBinding;
    VK_ZERO_MEMORY(layoutBinding);
    layoutBinding.stageFlags = mShaderStage;

    mTexResourceCount = 0;
    bool setApplied = false;
    for (uint32 i = 0; i < desc.numBindings; ++i)
    {
        ResourceBindingDesc rb = desc.resourceBindings[i];

        // decode set and bind from slot value
        // In contrast to D3D12, Vulkan connects all resources using a pair of numbers "set-binding inside set".
        // To make this work we heavily rely on Shader extracting Resource slot by name - in our case it is encoded
        // on a 32-bit value by splitting it into two parts. We need these values now because afterwards we have to
        // assign Descriptor Set Layouts created here in a proper order while creating a Pipeline Layout.
        // TODO MAYBE this could be avoided by creating the Pipeline Layout at a different stage? needs investigation
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
        else if (rb.resourceType == ShaderResourceType::Texture) // && WritableTexture?
        {
            NFE_LOG_WARNING("Sampler not provided for Texture Resource Type - binding default sampler");
            layoutBinding.pImmutableSamplers = &gDevice->GetDefaultSampler();
        }

        if (rb.resourceType == ShaderResourceType::Texture || rb.resourceType == ShaderResourceType::WritableTexture)
            mTexResourceCount++;

        bindings.PushBack(layoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo descInfo;
    VK_ZERO_MEMORY(descInfo);
    descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descInfo.bindingCount = bindings.Size();
    descInfo.pBindings = bindings.Data();
    VkResult result = vkCreateDescriptorSetLayout(gDevice->GetDevice(), &descInfo, nullptr, &mDescriptorLayout);
    CHECK_VKRESULT(result, "Failed to create Descriptor Set Layout");

    Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mDescriptorLayout), static_cast<VkObjectType>(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT), "DescriptorSetLayout");

    mResourceCount = bindings.Size();

    return true;
}

bool ResourceBindingSet::InitVolatile(ShaderType visibility, uint16 setSlot, uint16 binding, ShaderResourceType resType)
{
    NFE_UNUSED(resType);

    mShaderStage = TranslateShaderTypeToVkShaderStage(visibility);
    VkDescriptorSetLayoutBinding layoutBinding;
    VK_ZERO_MEMORY(layoutBinding);
    layoutBinding.stageFlags = mShaderStage;
    layoutBinding.binding = binding;
    layoutBinding.descriptorCount = 1;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

    VkDescriptorSetLayoutCreateInfo descInfo;
    VK_ZERO_MEMORY(descInfo);
    descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descInfo.bindingCount = 1;
    descInfo.pBindings = &layoutBinding;
    VkResult result = vkCreateDescriptorSetLayout(gDevice->GetDevice(), &descInfo, nullptr, &mDescriptorLayout);
    CHECK_VKRESULT(result, "Failed to create Descriptor Set Layout");

    Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mDescriptorLayout), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "DescriptorSetLayout");

    mResourceCount = 1;
    mTexResourceCount = 0;
    mSetSlot = setSlot;

    return true;
}


ResourceBindingLayout::ResourceBindingLayout()
    : mPipelineLayout(VK_NULL_HANDLE)
    , mBindingSets()
    , mVolatileBufferBindingSet()
    , mVolatileBufferSet(VK_NULL_HANDLE)
    , mVolatileBufferSetSlot(UINT16_MAX)
{
}

ResourceBindingLayout::~ResourceBindingLayout()
{
    if (mVolatileBufferSet != VK_NULL_HANDLE)
        vkFreeDescriptorSets(gDevice->GetDevice(), gDevice->GetDescriptorPool(), 1, &mVolatileBufferSet);
    mBindingSets.Clear();
    mVolatileBufferBindingSet.Reset();
    if (mPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(gDevice->GetDevice(), mPipelineLayout, nullptr);
}

bool ResourceBindingLayout::Init(const ResourceBindingLayoutDesc& desc)
{
    VkResult result = VK_SUCCESS;

    Common::DynArray<VkDescriptorSetLayout> dsls;
    const uint32 totalSetCount = desc.numBindingSets + desc.numVolatileCBuffers;
    dsls.Resize(totalSetCount);
    mBindingSets.Resize(totalSetCount);
    for (uint32 i = 0; i < desc.numBindingSets; ++i)
    {
        Common::SharedPtr<ResourceBindingSet> rbs = Common::DynamicCast<ResourceBindingSet>(desc.bindingSets[i]);
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

        mBindingSets[rbs->mSetSlot] = rbs;
        dsls[rbs->mSetSlot] = rbs->mDescriptorLayout;
    }

    if (desc.numVolatileCBuffers > 0)
    {
        // TODO support multiple
        const VolatileCBufferBinding& vb = desc.volatileCBuffers[0];

        mVolatileBufferSetSlot = (vb.slot >> 16);
        mVolatileBufferBindingSet = Common::MakeSharedPtr<ResourceBindingSet>();
        if (!mVolatileBufferBindingSet->InitVolatile(vb.shaderVisibility, mVolatileBufferSetSlot, vb.slot & 0xFFFF, vb.resourceType))
        {
            NFE_LOG_ERROR("Failed to create Volatile Buffer RBS");
            return false;
        }

        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mVolatileBufferBindingSet->mDescriptorLayout),
                                        VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "VolatileBufferDSL");

        mBindingSets[mVolatileBufferSetSlot] = mVolatileBufferBindingSet;
        dsls[mVolatileBufferSetSlot] = mVolatileBufferBindingSet->mDescriptorLayout;
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
        allocInfo.pSetLayouts = &mVolatileBufferBindingSet->mDescriptorLayout;
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


ResourceBindingInstance::~ResourceBindingInstance()
{
    for (auto& tv: mTextureViews)
    {
        if (tv != VK_NULL_HANDLE)
            vkDestroyImageView(gDevice->GetDevice(), tv, nullptr);
    }
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

    mWrittenResources.Resize(mSet->mResourceCount);
    for (auto& r: mWrittenResources)
        r = nullptr;

    mTextureViews.Resize(mSet->mTexResourceCount);
    for (auto& tv: mTextureViews)
        tv = VK_NULL_HANDLE;

    mImageInfos.Reserve(mSet->mResourceCount);
    mBufferInfos.Reserve(mSet->mResourceCount);
    mWrites.Reserve(mSet->mResourceCount);

    return true;
}

bool ResourceBindingInstance::SetTextureView(uint32 slot, const TexturePtr& texture, const TextureView& view)
{
    Texture* t = dynamic_cast<Texture*>(texture.Get());
    if (t == nullptr)
    {
        NFE_LOG_ERROR("Incorrect Texture pointer provided");
        return false;
    }

    VkImageView viewToUse = VK_NULL_HANDLE;
    if (t->mImageSubresRange == view)
    {
        viewToUse = t->mImageView;
    }
    else
    {
        VkImageSubresourceRange range = Util::TextureViewToVkImageSubresourceRange(view, t->mImageSubresRange.aspectMask);

        VkImageViewCreateInfo info;
        VK_ZERO_MEMORY(info);
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = t->mImage;
        info.viewType = Util::TextureTypeToVkImageViewType(t->mType);
        info.format = t->mFormat;
        info.components.r = VK_COMPONENT_SWIZZLE_R;
        info.components.g = VK_COMPONENT_SWIZZLE_G;
        info.components.b = VK_COMPONENT_SWIZZLE_B;
        info.components.a = VK_COMPONENT_SWIZZLE_A;
        info.subresourceRange = range;
        VkResult result = vkCreateImageView(gDevice->GetDevice(), &info, nullptr, &viewToUse);
        CHECK_VKRESULT(result, "Failed to create custom View for Image attached to resource");

        // store so we can free it later
        mTextureViews[slot] = viewToUse;
    }

    VkDescriptorImageInfo imgInfo;
    VK_ZERO_MEMORY(imgInfo);
    imgInfo.imageView = viewToUse;
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    mImageInfos.PushBack(imgInfo);

    VkWriteDescriptorSet writeSet;
    VK_ZERO_MEMORY(writeSet);
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = mDescriptorSet;
    writeSet.dstBinding = slot;
    writeSet.dstArrayElement = 0;
    writeSet.descriptorCount = 1;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeSet.pImageInfo = &mImageInfos.Back();

    mWrites.PushBack(writeSet);
    mWrittenResources.PushBack(t);

    return true;
}

bool ResourceBindingInstance::SetBufferView(uint32 slot, const BufferPtr& buffer, const BufferView& view)
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
    bufInfo.offset = b->mStructureSize * view.firstElement;
    bufInfo.range = b->mStructureSize * view.numElements;

    mBufferInfos.PushBack(bufInfo);

    VkWriteDescriptorSet writeSet;
    VK_ZERO_MEMORY(writeSet);
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = mDescriptorSet;
    writeSet.dstBinding = slot;
    writeSet.dstArrayElement = 0;
    writeSet.descriptorCount = 1;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeSet.pBufferInfo = &mBufferInfos.Back();

    mWrites.PushBack(writeSet);
    mWrittenResources.PushBack(b);

    return true;
}

bool ResourceBindingInstance::SetCBufferView(uint32 slot, const BufferPtr& buffer)
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

    mBufferInfos.PushBack(bufInfo);

    VkWriteDescriptorSet writeSet;
    VK_ZERO_MEMORY(writeSet);
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = mDescriptorSet;
    writeSet.dstBinding = slot;
    writeSet.dstArrayElement = 0;
    writeSet.descriptorCount = 1;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeSet.pBufferInfo = &mBufferInfos.Back();

    mWrites.PushBack(writeSet);
    mWrittenResources.PushBack(b);

    return true;
}

bool ResourceBindingInstance::SetWritableTextureView(uint32 slot, const TexturePtr& texture, const TextureView& view)
{
    NFE_UNUSED(slot);
    NFE_UNUSED(texture);
    NFE_UNUSED(view);

    return false;
}

bool ResourceBindingInstance::SetWritableBufferView(uint32 slot, const BufferPtr& buffer, const BufferView& view)
{
    NFE_UNUSED(slot);
    NFE_UNUSED(buffer);
    NFE_UNUSED(view);

    return false;
}

bool ResourceBindingInstance::Finalize()
{
    vkUpdateDescriptorSets(gDevice->GetDevice(), mWrites.Size(), mWrites.Data(), 0, nullptr);

    mImageInfos.Clear();
    mBufferInfos.Clear();
    mWrites.Clear();

    return true;
}

} // namespace Renderer
} // namespace NFE

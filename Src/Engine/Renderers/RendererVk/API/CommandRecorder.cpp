/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Command Recorder
 */

#include "PCH.hpp"

#include "CommandRecorder.hpp"
#include "Device.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Sampler.hpp"
#include "Backbuffer.hpp"
#include "PipelineState.hpp"
#include "ComputePipelineState.hpp"

#include "Internal/Translations.hpp"
#include "Internal/Debugger.hpp"

#include "Engine/Common/Math/Vec4fU.hpp"

#include <string.h>


namespace NFE {
namespace Renderer {

CommandRecorder::CommandRecorder()
    : mCommandBuffer(VK_NULL_HANDLE)
    , mQueueType(CommandQueueType::Invalid)
    , mCommandBufferBeginInfo()
    , mPipelineState(nullptr)
    , mPipelineInvalidated(false)
    , mPendingResources()
    , mPendingVolatileWrites()
    , mUsedDescriptorSets()
    , mVolatileResources()
    , mVolatileBufferOffsets()
    , mRenderTarget(nullptr)
    , mActiveRenderPass(false)
{
}

CommandRecorder::~CommandRecorder()
{
}


// PRIVATE METHODS //

void CommandRecorder::EnsureOutsideRenderPass()
{
    if (mActiveRenderPass)
    {
        vkCmdEndRenderPass(mCommandBuffer);

        mActiveRenderPass = false;
    }
}

void CommandRecorder::EnsureInsideRenderPass()
{
    if (!mActiveRenderPass)
    {
        NFE_ASSERT(mRenderTarget, "Render Target was not set!");

        VkRenderPassBeginInfo rpBeginInfo;
        VK_ZERO_MEMORY(rpBeginInfo);
        rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBeginInfo.renderPass = mRenderTarget->mRenderPass;
        rpBeginInfo.framebuffer = mRenderTarget->mFramebuffer;
        rpBeginInfo.renderArea.offset = { 0, 0 };
        rpBeginInfo.renderArea.extent = { static_cast<uint32>(mRenderTarget->mWidth),
                                          static_cast<uint32>(mRenderTarget->mHeight) };
        vkCmdBeginRenderPass(mCommandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        mActiveRenderPass = true;
    }
}

bool CommandRecorder::WriteDynamicBuffer(Buffer* b, size_t offset, size_t size, const void* data)
{
    // copy data to Ring Buffer
    uint32 sourceOffset = gDevice->GetRingBuffer()->Write(data, static_cast<uint32>(size));

    if (sourceOffset == UINT32_MAX)
    {
        NFE_LOG_ERROR("Failed to write temporary data to Ring Ruffer - the Ring Buffer is full");
        return false;
    }

    // CopyBuffer has to be called outside a Render Pass
    EnsureOutsideRenderPass();

    VkBufferCopy region;
    VK_ZERO_MEMORY(region);
    region.size = size;
    region.srcOffset = static_cast<VkDeviceSize>(sourceOffset);
    region.dstOffset = static_cast<VkDeviceSize>(offset);
    vkCmdCopyBuffer(mCommandBuffer, gDevice->GetRingBuffer()->GetVkBuffer(), b->mBuffer, 1, &region);

    return true;
}

bool CommandRecorder::WriteVolatileBuffer(Buffer* b, size_t size, const void* data)
{
    uint32 offset = gDevice->GetRingBuffer()->Write(data, static_cast<uint32>(size));
    if (offset == UINT32_MAX)
    {
        NFE_LOG_ERROR("Failed to write data to Ring Ruffer - the Ring Buffer is full");
        return false;
    }

    mPendingVolatileWrites.EmplaceBack(b, offset);
    return true;
}

void CommandRecorder::UpdateVolatileResources(BasePipelineState* oldState)
{
    if (oldState == nullptr)
    {
        mVolatileResources.Resize(mPipelineState->GetVolatileResourceCount());
        return;
    }

    if (mPipelineState->GetVolatileResourceCount() == 0)
    {
        mVolatileResources.Clear();
        return;
    }

    VolatileResources oldResources(mVolatileResources);
    mVolatileResources.Resize(mPipelineState->GetVolatileResourceCount());
    for (uint32 i = 0; i < mVolatileResources.Size(); ++i)
        mVolatileResources[i] = nullptr;

    if (oldResources.Empty())
        return;

    // Copy over volatile resources from old to new collection, but only if their binding matches
    const BasePipelineState::VolatileResourceMetadata& oldVRM = oldState->GetVolatileResources();
    const BasePipelineState::VolatileResourceMetadata& newVRM = mPipelineState->GetVolatileResources();
    for (uint32 oldIdx = 0; oldIdx < oldVRM.Size(); ++oldIdx)
    {
        for (uint32 newIdx = 0; newIdx < newVRM.Size(); ++newIdx)
        {
            if (oldVRM[oldIdx].stage == newVRM[newIdx].stage &&
                oldVRM[oldIdx].binding == newVRM[newIdx].binding)
            {
                mVolatileResources[newIdx] = oldResources[oldIdx];
                break;
            }
        }
    }
}

void CommandRecorder::TransferResources(BasePipelineState* state, BasePipelineState* oldState)
{
    // TODO PLS
    NFE_UNUSED(state);
    NFE_UNUSED(oldState);
}

uint32 CommandRecorder::AcquireTargetDescriptorSetIdx(ShaderType stage, ShaderResourceType type)
{
    VkShaderStageFlagBits stageFlags = TranslateShaderTypeToVkShaderStage(stage);
    VkDescriptorType descType;

    switch (type)
    {
        case ShaderResourceType::UniformBuffer: descType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
        case ShaderResourceType::SampledImage: descType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
        case ShaderResourceType::Sampler: descType = VK_DESCRIPTOR_TYPE_SAMPLER; break;
        case ShaderResourceType::StorageBuffer: descType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
        case ShaderResourceType::StorageImage: descType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; break;
        case ShaderResourceType::UniformTexelBuffer: descType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER; break;
        case ShaderResourceType::StorageTexelBuffer: descType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER; break;
        default: return UINT32_MAX;
    }

    for (uint32 i = 0; i < mPipelineState->GetDescriptorSetCount(); ++i)
    {
        const BasePipelineState::DescriptorSetMetadataEntry& metadata = mPipelineState->GetDescriptorSet(i);
        // strip _DYNAMIC for easier comparisons
        VkDescriptorType metaTypeGeneral = (metadata.bindings[0].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ?
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : metadata.bindings[0].type;
        if (stageFlags == metadata.stage && descType == metaTypeGeneral)
            return metadata.set;
    }

    return UINT32_MAX;
}

void CommandRecorder::InsertVolatileResource(IResource* r, VkShaderStageFlagBits stage, uint32 slot)
{
    NFE_ASSERT(mPipelineState != nullptr, "Pipeline State not bound");

    NFE_ASSERT(mPipelineState->GetVolatileResourceCount() == mVolatileResources.Size(),
        "Volatile Resource storage size doesn't match Metadata size - is Pipeline State bound?");
    for (uint32 i = 0; i < mPipelineState->GetVolatileResourceCount(); ++i)
    {
        const BasePipelineState::VolatileResourceMetadataEntry vrm =
            mPipelineState->GetVolatileResource(i);
        if (vrm.stage == stage && vrm.binding == slot)
        {
            mVolatileResources[i] = r;
        }
    }
}

void CommandRecorder::SortPendingResources()
{
    if (mPendingResources.Size() < 2)
        return; // no reason to sort one resource (or no resources)

    // TODO replace with Common::Sort when it starts to exist

    // first, sort resources by stage they occur in
    std::sort(mPendingResources.begin(), mPendingResources.end(),
        [](const PendingResource& a, const PendingResource& b) -> bool {
            return a.stage < b.stage;
        }
    );

    // next, sort resources within each stage according to their slot number
    NFE::Common::ArrayIterator<PendingResource> stageBegin = mPendingResources.Begin();
    NFE::Common::ArrayIterator<PendingResource> stageEnd = stageBegin;

    ++stageEnd;
    while (stageEnd != mPendingResources.End())
    {
        if (stageEnd->stage != stageBegin->stage)
        {
            std::sort(stageBegin, stageEnd,
                [](const PendingResource& a, const PendingResource& b) -> bool {
                    return a.slot < b.slot;
                }
            );

            stageBegin = stageEnd;
        }

        ++stageEnd;
    }
}

void CommandRecorder::ProcessPendingResources()
{
    if (mPendingResources.Empty())
        return;

    DescriptorSetCollectionID dsID = gDevice->GetDescriptorSetCache().AllocateDescriptorSets(mPipelineState->GetDescriptorSetLayouts());
    DescriptorSetCollection& sets = gDevice->GetDescriptorSetCache().GetDescriptorSets(dsID);

    if (!mUsedDescriptorSets.Empty())
    {
        // copy sets from previously used DS - we will update over them
        // TODO this might be a lil bit less memory intensive:
        //  - Hold a "main version" of Descriptor Sets used by CR
        //  - If we have pending resources, apply a "delta" and update only these sets
        //    that require new resources bound to them.
        //  - In BindDescriptorSets() form a new array of Sets which is a mixture of
        //    "latest updated" sets.
        DescriptorSetCollection& prevSets =
            gDevice->GetDescriptorSetCache().GetDescriptorSets(mUsedDescriptorSets.Back());

        Common::StaticArray<VkCopyDescriptorSet, VK_MAX_DESCRIPTOR_SETS> copySets;

        for (uint32 i = 0; i < prevSets.Size(); ++i)
        {
            // FIXME this is actually wrong, because it assumes the Pipeline is the same.
            // In the meantime, in between draw calls the Pipeline State could change.
            // Moreover, there is an expectation to preserve previous binds at the new
            // Pipeline. Fix this?
            VkCopyDescriptorSet copySet;
            VK_ZERO_MEMORY(copySet);
            copySet.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
            copySet.descriptorCount = mPipelineState->GetDescriptorSet(i).bindings.Size();
            copySet.srcSet = sets[i];
            copySet.srcBinding = 0;
            copySet.dstSet = prevSets[i];
            copySet.dstBinding = 0;

            copySets.EmplaceBack(copySet);
        }

        vkUpdateDescriptorSets(gDevice->GetDevice(), 0, nullptr, copySets.Size(), copySets.Data());
    }

    mUsedDescriptorSets.EmplaceBack(dsID);

    Common::StaticArray<VkDescriptorBufferInfo, VK_MAX_PENDING_RESOURCES> bufferInfos;
    Common::StaticArray<VkDescriptorImageInfo, VK_MAX_PENDING_RESOURCES> imageInfos;
    Common::StaticArray<VkBufferView, VK_MAX_PENDING_RESOURCES> texelBufferViews;
    Common::StaticArray<VkWriteDescriptorSet, VK_MAX_PENDING_RESOURCES> writeSets;

    for (uint32 i = 0; i < mPendingResources.Size(); ++i)
    {
        // TODO check if types match, don't write/bind if not

        bool isBufferWrite = false;
        bool isImageWrite = false;
        bool isTexelBufferWrite = false;

        switch (mPendingResources[i].type)
        {
        case ShaderResourceType::UniformBuffer:
        case ShaderResourceType::StorageBuffer:
        {
            bufferInfos.EmplaceBack();
            VkDescriptorBufferInfo& bufferInfo = bufferInfos.Back();

            Buffer* b = dynamic_cast<Buffer*>(mPendingResources[i].resource);

            if (b->mMode == ResourceAccessMode::Volatile)
                InsertVolatileResource(b, TranslateShaderTypeToVkShaderStage(mPendingResources[i].stage), mPendingResources[i].slot);

            VK_ZERO_MEMORY(bufferInfo);
            bufferInfo.buffer = (b->mMode == ResourceAccessMode::Volatile) ? gDevice->GetRingBuffer()->GetVkBuffer() : b->mBuffer;
            bufferInfo.range = b->mBufferSize;
            bufferInfo.offset = 0;

            isBufferWrite = true;
            break;
        }
        case ShaderResourceType::UniformTexelBuffer:
        case ShaderResourceType::StorageTexelBuffer:
        {
            Buffer* b = dynamic_cast<Buffer*>(mPendingResources[i].resource);
            texelBufferViews.EmplaceBack(b->mView);
            isTexelBufferWrite = true;
            break;
        }
        case ShaderResourceType::SampledImage:
        case ShaderResourceType::StorageImage:
        {
            imageInfos.EmplaceBack();
            VkDescriptorImageInfo& imageInfo = imageInfos.Back();

            VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            if (mPendingResources[i].type == ShaderResourceType::StorageImage)
                layout = VK_IMAGE_LAYOUT_GENERAL;

            Texture* t = dynamic_cast<Texture*>(mPendingResources[i].resource);
            VK_ZERO_MEMORY(imageInfo);
            imageInfo.imageView = t->mImageView;
            imageInfo.imageLayout = layout;
            imageInfo.sampler = nullptr;

            isImageWrite = true;
            break;
        }
        case ShaderResourceType::Sampler:
        {
            imageInfos.EmplaceBack();
            VkDescriptorImageInfo& imageInfo = imageInfos.Back();

            Sampler* s = dynamic_cast<Sampler*>(mPendingResources[i].resource);
            VK_ZERO_MEMORY(imageInfo);
            imageInfo.sampler = s->mSampler;

            isImageWrite = true;
            break;
        }
        default:
            NFE_LOG_WARNING("Invalid resource type for pending resource bind, skipping");
            continue;
        }

        uint32 setIdx = AcquireTargetDescriptorSetIdx(mPendingResources[i].stage, mPendingResources[i].type);

        VkWriteDescriptorSet writeSet;
        VK_ZERO_MEMORY(writeSet);
        writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSet.descriptorCount = 1;
        writeSet.descriptorType = mPipelineState->GetDescriptorType(setIdx, mPendingResources[i].slot);
        writeSet.dstSet = sets[setIdx];
        writeSet.dstBinding = mPendingResources[i].slot;
        writeSet.pBufferInfo = isBufferWrite ? &bufferInfos.Back() : nullptr;
        writeSet.pImageInfo = isImageWrite ? &imageInfos.Back() : nullptr;
        writeSet.pTexelBufferView = isTexelBufferWrite ? &texelBufferViews.Back() : nullptr;

        writeSets.EmplaceBack(writeSet);
    }

    vkUpdateDescriptorSets(gDevice->GetDevice(), writeSets.Size(), writeSets.Data(), 0, nullptr);

    mVolatileBufferOffsets.Resize(mVolatileResources.Size());

    mPendingResources.Clear();
}

void CommandRecorder::ProcessVolatileOffsets()
{
    for (const PendingVolatileWrite& write: mPendingVolatileWrites)
    {
        for (uint32 i = 0; i < mVolatileResources.Size(); ++i)
        {
            if (mVolatileResources[i] == write.resource)
            {
                mVolatileBufferOffsets[i] = write.offset;
            }
        }
    }

    mPendingVolatileWrites.Clear();
}

void CommandRecorder::BindDescriptorSets()
{
    if (mUsedDescriptorSets.Empty())
        return;

    DescriptorSetCollection& sets = gDevice->GetDescriptorSetCache().GetDescriptorSets(mUsedDescriptorSets.Back());

    vkCmdBindDescriptorSets(mCommandBuffer, mPipelineState->GetBindPoint(),
                            mPipelineState->GetPipelineLayout(), 0,
                            sets.Size(), sets.Data(),
                            mVolatileBufferOffsets.Size(), mVolatileBufferOffsets.Data());
}

void CommandRecorder::PreDraw()
{
    SortPendingResources();
    ProcessPendingResources();
    ProcessVolatileOffsets();
    BindDescriptorSets();

    EnsureInsideRenderPass();
}

void CommandRecorder::PreDispatch()
{
    SortPendingResources();
    ProcessPendingResources();
    BindDescriptorSets();
}


// INITIALIZATION

bool CommandRecorder::Init()
{
    VK_ZERO_MEMORY(mCommandBufferBeginInfo);
    mCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    mCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    NFE_LOG_INFO("Command Buffer initialized successfully.");
    return true;
}


// COMMON METHODS

bool CommandRecorder::Begin(CommandQueueType queueType)
{
    mRenderTarget = nullptr;
    mPipelineState = nullptr;

    // Get CommandBuffer from select queue
    mCommandBuffer = gDevice->GetCommandBufferManager(queueType).Acquire();
    mQueueType = queueType;
    VkResult result = vkBeginCommandBuffer(mCommandBuffer, &mCommandBufferBeginInfo);
    CHECK_VKRESULT(result, "Failed to begin Command Buffer");

    return true;
}

void CommandRecorder::CopyBuffer(const BufferPtr& src, const BufferPtr& dest, size_t size, size_t srcOffset, size_t destOffset)
{
    NFE_UNUSED(src);
    NFE_UNUSED(dest);
    NFE_UNUSED(size);
    NFE_UNUSED(srcOffset);
    NFE_UNUSED(destOffset);
    NFE_FATAL("Not yet implemented");
}

void CommandRecorder::CopyTexture(const TexturePtr& src, const TexturePtr& dest)
{
    NFE_UNUSED(src);
    NFE_UNUSED(dest);

    NFE_FATAL("Not yet implemented!");
}

void CommandRecorder::CopyTexture(const TexturePtr& src, const BackbufferPtr& dest)
{
    Texture* s = dynamic_cast<Texture*>(src.Get());
    Backbuffer* d = dynamic_cast<Backbuffer*>(dest.Get());

    NFE_ASSERT(s != nullptr, "Invalid source texture provided");
    NFE_ASSERT(d != nullptr, "Invalid destination backbuffer provided");

    EnsureOutsideRenderPass();

    gDevice->GetLayoutTracker().EnsureLayout(mCommandBuffer, s->GetID(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    gDevice->GetLayoutTracker().EnsureLayout(mCommandBuffer, d->GetCurrentResourceID(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    if ((s->mFormat == d->mFormat) && (s->mWidth == d->mWidth) && (s->mHeight == d->mHeight))
    {
        // do a raw copy, which doesn't do format conversion/scaling and is faster than blitting
        VkImageCopy copyRegion;
        VK_ZERO_MEMORY(copyRegion);
        copyRegion.srcSubresource.aspectMask = s->mImageSubresRange.aspectMask;
        copyRegion.srcSubresource.baseArrayLayer = s->mImageSubresRange.baseArrayLayer;
        copyRegion.srcSubresource.layerCount = s->mImageSubresRange.layerCount;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.dstSubresource.aspectMask = d->mImageSubresRange.aspectMask;
        copyRegion.dstSubresource.baseArrayLayer = d->mImageSubresRange.baseArrayLayer;
        copyRegion.dstSubresource.layerCount = d->mImageSubresRange.layerCount;
        copyRegion.dstSubresource.mipLevel = 0;
        copyRegion.extent.width = s->mWidth;
        copyRegion.extent.height = s->mHeight;
        copyRegion.extent.depth = 1;
        vkCmdCopyImage(mCommandBuffer,
                       s->mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       d->mImages[d->mCurrentImage], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &copyRegion);
    }
    else
    {
        // Blit the image
        VkImageBlit blitRegion;
        VK_ZERO_MEMORY(blitRegion);
        // subresources specification
        blitRegion.srcSubresource.aspectMask = s->mImageSubresRange.aspectMask;
        blitRegion.srcSubresource.baseArrayLayer = s->mImageSubresRange.baseArrayLayer;
        blitRegion.srcSubresource.layerCount = s->mImageSubresRange.layerCount;
        blitRegion.srcSubresource.mipLevel = 0;
        blitRegion.dstSubresource.aspectMask = d->mImageSubresRange.aspectMask;
        blitRegion.dstSubresource.baseArrayLayer = d->mImageSubresRange.baseArrayLayer;
        blitRegion.dstSubresource.layerCount = d->mImageSubresRange.layerCount;
        blitRegion.dstSubresource.mipLevel = 0;
        // source offsets - define region to copy from
        blitRegion.srcOffsets[0].x = 0;
        blitRegion.srcOffsets[0].y = 0;
        blitRegion.srcOffsets[0].z = 0;
        blitRegion.srcOffsets[1].x = s->mWidth;
        blitRegion.srcOffsets[1].y = s->mHeight;
        blitRegion.srcOffsets[1].z = 1;
        // destination offsets - define region where to copy to
        blitRegion.dstOffsets[0].x = 0;
        blitRegion.dstOffsets[0].y = 0;
        blitRegion.dstOffsets[0].z = 0;
        blitRegion.dstOffsets[1].x = d->mWidth;
        blitRegion.dstOffsets[1].y = d->mHeight;
        blitRegion.dstOffsets[1].z = 1;
        vkCmdBlitImage(mCommandBuffer,
                       s->mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       d->mImages[d->mCurrentImage], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blitRegion, VK_FILTER_NEAREST);
    }

    // TODO HACK
    gDevice->GetLayoutTracker().EnsureLayout(mCommandBuffer, d->GetCurrentResourceID(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

void CommandRecorder::CopyTextureToBuffer(const TexturePtr& src, const BufferPtr& dest, const TextureRegion* texRegion, uint32 bufferOffset)
{
    NFE_UNUSED(src);
    NFE_UNUSED(dest);
    NFE_UNUSED(texRegion);
    NFE_UNUSED(bufferOffset);
    NFE_FATAL("Not yet implemented");
}

bool CommandRecorder::WriteBuffer(const BufferPtr& buffer, size_t offset, size_t size, const void* data)
{
    Buffer* b = dynamic_cast<Buffer*>(buffer.Get());
    if (b == nullptr)
    {
        NFE_LOG_ERROR("Invalid buffer pointer provided");
        return false;
    }

    if (b->mMode == ResourceAccessMode::GPUOnly || b->mMode == ResourceAccessMode::Immutable)
    {
        if (static_cast<VkDeviceSize>(size) > b->mBufferSize)
        {
            NFE_LOG_ERROR("Requested to write more than provided buffer can handle (%d vs buffer's %d)",
                      size, b->mBufferSize);
            return false;
        }

        return WriteDynamicBuffer(b, offset, size, data);
    }
    else if (b->mMode == ResourceAccessMode::Volatile)
    {
        return WriteVolatileBuffer(b, size, data);
    }
    else
    {
        NFE_LOG_ERROR("Provided buffer does not have a CPU-writable mode");
        return false;
    }
}

bool CommandRecorder::WriteTexture(const TexturePtr& texture, const void* data, const TextureRegion* texRegion, uint32 srcRowStride)
{
    Texture* tex = dynamic_cast<Texture*>(texture.Get());
    NFE_ASSERT(tex != nullptr, "Invalid texture ptr");
    NFE_ASSERT(data != nullptr, "Invalid data ptr");

    uint32 texSize;
    uint32 offsetLinear;
    if (texRegion != nullptr)
    {
        texSize = texRegion->width * texRegion->height * texRegion->depth;
        offsetLinear = (tex->mWidth * tex->mHeight) * texRegion->z +
                       (tex->mWidth) * texRegion->y +
                        texRegion->x;
    }
    else
    {
        texSize = tex->mWidth * tex->mHeight * tex->mDepth;
        offsetLinear = 0;
    }

    uint32 bytesPerTexel = Util::GetVkFormatByteSize(tex->mFormat);
    texSize *= bytesPerTexel;
    offsetLinear *= bytesPerTexel;

    // TODO this is probably wrong and will result in a malformed texture if TextureRegion uses offsets
    const uint8* dataBytesPtr = reinterpret_cast<const uint8*>(data);
    uint32 textureLocation = gDevice->GetRingBuffer()->Write(dataBytesPtr + offsetLinear, texSize);
    if (textureLocation == UINT32_MAX)
    {
        NFE_LOG_ERROR("Failed to write texture data to Ring Buffer - out of memory");
        return false;
    }

    gDevice->GetLayoutTracker().EnsureLayout(mCommandBuffer, tex->GetID(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy2KHR region;
    VK_ZERO_MEMORY(region);
    region.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2_KHR;
    region.bufferOffset = textureLocation;
    if (srcRowStride)
    {
        region.bufferImageHeight = srcRowStride * (texRegion->height - texRegion->y);
        region.bufferRowLength = srcRowStride;
    }
    else
    {
        region.bufferImageHeight = tex->mHeight;
        region.bufferRowLength = tex->mWidth;
    }
    if (texRegion != nullptr)
    {
        region.imageOffset.x = texRegion->x;
        region.imageOffset.y = texRegion->y;
        region.imageOffset.z = texRegion->z;
        region.imageExtent.width = texRegion->width;
        region.imageExtent.height = texRegion->height;
        region.imageExtent.depth = texRegion->depth;
        region.imageSubresource.mipLevel = texRegion->mipmap;
        region.imageSubresource.baseArrayLayer = texRegion->layer;
    }
    else
    {
        region.imageOffset.x = 0;
        region.imageOffset.y = 0;
        region.imageOffset.z = 0;
        region.imageExtent.width = tex->mWidth;
        region.imageExtent.height = tex->mHeight;
        region.imageExtent.depth = tex->mDepth;
        region.imageSubresource.mipLevel = tex->mImageSubresRange.baseMipLevel;
        region.imageSubresource.baseArrayLayer = tex->mImageSubresRange.baseArrayLayer;
    }
    region.imageSubresource.aspectMask = tex->mImageSubresRange.aspectMask;
    region.imageSubresource.layerCount = 1;

    VkCopyBufferToImageInfo2KHR copyInfo;
    VK_ZERO_MEMORY(copyInfo);
    copyInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2_KHR;
    copyInfo.srcBuffer = gDevice->GetRingBuffer()->GetVkBuffer();
    copyInfo.dstImage = tex->mImage;
    copyInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    copyInfo.regionCount = 1;
    copyInfo.pRegions = &region;
    vkCmdCopyBufferToImage2KHR(mCommandBuffer, &copyInfo);

    return true;
}

CommandListPtr CommandRecorder::Finish()
{
    EnsureOutsideRenderPass();

    VkResult result = vkEndCommandBuffer(mCommandBuffer);
    if (result != VK_SUCCESS)
    {
        NFE_LOG_ERROR("Error during Constant Buffer recording: %d (%s)",
                  result, TranslateVkResultToString(result));
        return 0;
    }

    // mCommandBuffer is now owned by CommandList
    CommandListPtr cl = gDevice->CreateCommandList(mQueueType, mCommandBuffer, mUsedDescriptorSets);
    mCommandBuffer = VK_NULL_HANDLE;
    mQueueType = CommandQueueType::Invalid;
    mUsedDescriptorSets.Clear();
    return cl;
}


// GRAPHICS PIPELINE METHODS //

void CommandRecorder::BindBuffer(ShaderType stage, uint32 slot, const BufferPtr& buffer, const BufferView& view)
{
    // TODO
    NFE_UNUSED(view);

    Buffer* b = dynamic_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(b != nullptr, "Invalid Buffer pointer");

    ShaderResourceType srt = ShaderResourceType::UniformBuffer;
    if (HasFlag(b->GetUsage(), BufferUsageFlag::ReadonlyBuffer))
        srt = ShaderResourceType::UniformTexelBuffer;

    mPendingResources.EmplaceBack(b, stage, srt, slot);
}

void CommandRecorder::BindConstantBuffer(ShaderType stage, uint32 slot, const BufferPtr& buffer)
{
    Buffer* b = dynamic_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(b != nullptr, "Invalid Buffer pointer");

    mPendingResources.EmplaceBack(b, stage, ShaderResourceType::UniformBuffer, slot);
}

void CommandRecorder::BindTexture(ShaderType stage, uint32 slot, const TexturePtr& texture, const TextureView& view)
{
    // TODO
    NFE_UNUSED(view);

    EnsureOutsideRenderPass();
    Texture* t = dynamic_cast<Texture*>(texture.Get());
    NFE_ASSERT(t != nullptr, "Invalid Texture pointer");

    gDevice->GetLayoutTracker().EnsureLayout(mCommandBuffer, t->GetID(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    mPendingResources.EmplaceBack(t, stage, ShaderResourceType::SampledImage, slot);
}

void CommandRecorder::BindSampler(ShaderType stage, uint32 slot, const SamplerPtr& sampler)
{
    Sampler* s = dynamic_cast<Sampler*>(sampler.Get());
    NFE_ASSERT(s != nullptr, "Invalid Sampler pointer");

    mPendingResources.EmplaceBack(s, stage, ShaderResourceType::Sampler, slot);
}

void CommandRecorder::BindWritableBuffer(ShaderType stage, uint32 slot, const BufferPtr& buffer, const BufferView& view)
{
    NFE_UNUSED(view);

    Buffer* b = dynamic_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(b != nullptr, "Invalid Buffer pointer");

    ShaderResourceType srt = ShaderResourceType::StorageBuffer;
    if (HasFlag(b->GetUsage(), BufferUsageFlag::WritableBuffer))
        srt = ShaderResourceType::StorageTexelBuffer;

    mPendingResources.EmplaceBack(b, stage, srt, slot);
}

void CommandRecorder::BindWritableTexture(ShaderType stage, uint32 slot, const TexturePtr& texture, const TextureView& view)
{
    // TODO
    NFE_UNUSED(view);

    EnsureOutsideRenderPass();
    Texture* t = dynamic_cast<Texture*>(texture.Get());
    NFE_ASSERT(t != nullptr, "Invalid Texture pointer");

    gDevice->GetLayoutTracker().EnsureLayout(mCommandBuffer, t->GetID(), VK_IMAGE_LAYOUT_GENERAL);
    mPendingResources.EmplaceBack(t, stage, ShaderResourceType::StorageImage, slot);
}

void CommandRecorder::Clear(uint32 flags, uint32 numTargets, const uint32* slots, const Math::Vec4fU* colors, float depthValue, uint8 stencilValue)
{
    if (!mRenderTarget)
        return;

    EnsureInsideRenderPass();

    VkClearAttachment clearAtts[MAX_RENDER_TARGETS + 2];
    uint32 clearAttsNum = 0;

    if (flags & ClearFlagsColor)
    {
        for (uint32 i = 0; i < numTargets; ++i)
        {
            uint32 slot = i;
            if (slots)
            {
                slot = slots[i];
            }

            clearAtts[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            clearAtts[i].colorAttachment = slot;
            clearAtts[i].clearValue.color.float32[0] = colors[i].x;
            clearAtts[i].clearValue.color.float32[1] = colors[i].y;
            clearAtts[i].clearValue.color.float32[2] = colors[i].z;
            clearAtts[i].clearValue.color.float32[3] = colors[i].w;
            clearAttsNum++;
        }
    }

    if (flags & ClearFlagsDepth)
    {
        clearAtts[clearAttsNum].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        clearAtts[clearAttsNum].clearValue.depthStencil.depth = depthValue;
        clearAttsNum++;
    }

    if (flags & ClearFlagsStencil)
    {
        clearAtts[clearAttsNum].aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        clearAtts[clearAttsNum].clearValue.depthStencil.stencil = stencilValue;
        clearAttsNum++;
    }

    VkClearRect rect;
    rect.rect.offset = { 0, 0 };
    rect.rect.extent = { static_cast<uint32>(mRenderTarget->mWidth),
                         static_cast<uint32>(mRenderTarget->mHeight) };
    rect.baseArrayLayer = 0;
    rect.layerCount = 1;
    vkCmdClearAttachments(mCommandBuffer, clearAttsNum, clearAtts, 1, &rect);
}

void CommandRecorder::SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format)
{
    Buffer* ib = dynamic_cast<Buffer*>(indexBuffer.Get());
    NFE_ASSERT(ib != nullptr, "Incorrect Index Buffer provided");

    vkCmdBindIndexBuffer(mCommandBuffer, ib->mBuffer, 0, TranslateIndexBufferFormatToVkIndexType(format));
}

void CommandRecorder::SetRenderTarget(const RenderTargetPtr& renderTarget)
{
    EnsureOutsideRenderPass();

    if (renderTarget == nullptr)
    {
        mRenderTarget = nullptr;
        return;
    }

    mRenderTarget = dynamic_cast<RenderTarget*>(renderTarget.Get());

    LayoutTracker& tracker = gDevice->GetLayoutTracker();
    for (uint32 i = 0; i < mRenderTarget->mAttachments.Size(); ++i)
    {
        tracker.EnsureLayout(mCommandBuffer, mRenderTarget->mAttachments[i]->GetID(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    if (mRenderTarget->mDepthAttachment != nullptr)
    {
        tracker.EnsureLayout(mCommandBuffer, mRenderTarget->mDepthAttachment->GetID(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
}

void CommandRecorder::SetPipelineState(const PipelineStatePtr& state)
{
    NFE_ASSERT(state, "Cannot set null pipeline state");

    BasePipelineState* oldState = mPipelineState;
    mPipelineState = dynamic_cast<BasePipelineState*>(state.Get());
    NFE_ASSERT(mPipelineState != nullptr, "Invalid Pipeline State provided");

    vkCmdBindPipeline(mCommandBuffer, mPipelineState->GetBindPoint(), mPipelineState->GetPipeline());
    TransferResources(mPipelineState, oldState);
    UpdateVolatileResources(oldState);
}

void CommandRecorder::SetComputePipelineState(const ComputePipelineStatePtr& state)
{
    BasePipelineState* oldState = mPipelineState;
    mPipelineState = dynamic_cast<BasePipelineState*>(state.Get());
    NFE_ASSERT(mPipelineState != nullptr, "Invalid Compute Pipeline State provided");

    vkCmdBindPipeline(mCommandBuffer, mPipelineState->GetBindPoint(), mPipelineState->GetPipeline());
    TransferResources(mPipelineState, oldState);
}

void CommandRecorder::SetScissors(int32 left, int32 top, int32 right, int32 bottom)
{
    VkRect2D scissor;
    scissor.offset.x = left;
    scissor.offset.y = top;
    scissor.extent.width = right - left;
    scissor.extent.height = bottom - top;
    vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
}

void CommandRecorder::SetStencilRef(unsigned char ref)
{
    vkCmdSetStencilReference(mCommandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, ref);
}

void CommandRecorder::SetVertexBuffers(uint32 num, const BufferPtr* vertexBuffers, const uint32* strides, const uint32* offsets)
{
    NFE_UNUSED(strides);

    const uint32 maxBuffers = 4;
    VkBuffer buffers[maxBuffers];
    VkDeviceSize offs[maxBuffers];

    for (uint32 i = 0; i < num; ++i)
    {
        Buffer* buf = dynamic_cast<Buffer*>(vertexBuffers[i].Get());
        NFE_ASSERT(buf != nullptr, "Incorrect buffer provided at slot %d", i);

        if (buf->mMode == ResourceAccessMode::Volatile)
        {
            buffers[i] = gDevice->GetRingBuffer()->GetVkBuffer();
            offs[i] = static_cast<VkDeviceSize>(offsets[i]) + buf->mVolatileDataOffset;
        }
        else
        {
            buffers[i] = buf->mBuffer;
            offs[i] = static_cast<VkDeviceSize>(offsets[i]);
        }
    }

    // TODO assumes start slot 0
    vkCmdBindVertexBuffers(mCommandBuffer, 0, num, buffers, offs);
}

void CommandRecorder::SetViewport(float left, float width, float top, float height, float minDepth, float maxDepth)
{
    VkViewport viewport;
    viewport.x = left;
    viewport.y = (height - top);
    viewport.width = width;
    viewport.height = -height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
}

void CommandRecorder::Draw(uint32 vertexNum, uint32 instancesNum, uint32 vertexOffset, uint32 instanceOffset)
{
    PreDraw();
    vkCmdDraw(mCommandBuffer, vertexNum, instancesNum, vertexOffset, instanceOffset);
}

void CommandRecorder::DrawIndexed(uint32 indexNum, uint32 instancesNum, uint32 indexOffset, int32 vertexOffset, uint32 instanceOffset)
{
    PreDraw();
    vkCmdDrawIndexed(mCommandBuffer, indexNum, instancesNum, indexOffset, vertexOffset, instanceOffset);
}

void CommandRecorder::Dispatch(uint32 x, uint32 y, uint32 z)
{
    PreDispatch();
    vkCmdDispatch(mCommandBuffer, x, y, z);
}

void CommandRecorder::DispatchIndirect(const BufferPtr& indirectArgBuffer, uint32 bufferOffset)
{
    Buffer* b = dynamic_cast<Buffer*>(indirectArgBuffer.Get());
    NFE_ASSERT(b != nullptr, "Incorrect Indirect Arg Buffer provided");

    PreDispatch();
    vkCmdDispatchIndirect(mCommandBuffer, b->mBuffer, bufferOffset);
}


// HINTS/RESOURCE QUEUE TRANSITIONS

void CommandRecorder::HintTargetCommandQueueType(const BufferPtr& resource, const CommandQueueType targetType)
{
    NFE_UNUSED(resource);
    NFE_UNUSED(targetType);
    NFE_FATAL("Not yet implemented");
}

void CommandRecorder::HintTargetCommandQueueType(const TexturePtr& resource, const CommandQueueType targetType)
{
    NFE_UNUSED(resource);
    NFE_UNUSED(targetType);
    NFE_FATAL("Not yet implemented");
}


// DEBUGGING

void CommandRecorder::BeginDebugGroup(const char* text)
{
    NFE_UNUSED(text);
}

void CommandRecorder::EndDebugGroup()
{
}

void CommandRecorder::InsertDebugMarker(const char* text)
{
    NFE_UNUSED(text);
}

} // namespace Renderer
} // namespace NFE

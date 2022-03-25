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
    , mPendingResources()
    , mTemporaryDescriptorSets()
    , mRenderTarget(nullptr)
    , mActiveRenderPass(false)
    , mBoundVolatileBuffers()
    , mBoundVolatileOffsets()
    , mRebindDynamicBuffers(false)
{
}

CommandRecorder::~CommandRecorder()
{
    ClearDescriptorSetBindings();
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

    // CopyBuffer has to be call outside a Render Pass
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
    b->mVolatileDataOffset = gDevice->GetRingBuffer()->Write(data, static_cast<uint32>(size));
    NFE_ASSERT(b->mVolatileDataOffset != UINT32_MAX, "Failed to write data to Ring Ruffer - the Ring Buffer is full");

    for (uint32 i = 0; i < VK_MAX_VOLATILE_BUFFERS; ++i)
    {
        if (b == mBoundVolatileBuffers[i])
        {
            mBoundVolatileOffsets[i] = b->mVolatileDataOffset;
            mRebindDynamicBuffers = true;
            break;
        }
    }

    return true;
}

void CommandRecorder::ClearDescriptorSetBindings()
{
    if (mTemporaryDescriptorSets.Size() > 0)
        vkFreeDescriptorSets(gDevice->GetDevice(), gDevice->GetDescriptorPool(),
                            mTemporaryDescriptorSets.Size(), mTemporaryDescriptorSets.Data());
    mTemporaryDescriptorSets.Clear();
}

uint32 CommandRecorder::AcquireTargetDescriptorSetIdx(ShaderType stage, ShaderResourceType type)
{
    VkShaderStageFlags stageFlags = TranslateShaderTypeToVkShaderStage(stage);
    VkDescriptorType descType;

    switch (type)
    {
        case ShaderResourceType::UniformBuffer: descType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
        case ShaderResourceType::SampledImage: descType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
        case ShaderResourceType::Sampler: descType = VK_DESCRIPTOR_TYPE_SAMPLER; break;
        case ShaderResourceType::StorageBuffer: descType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
        case ShaderResourceType::StorageImage: descType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; break;
        default: return UINT32_MAX;
    }

    for (uint32 i = 0; i < mPipelineState->mDescriptorSetMetadata.Size(); ++i)
    {
        const PipelineState::DescriptorSetMetadata& metadata = mPipelineState->mDescriptorSetMetadata[i];
        if (stageFlags == metadata.stage && descType == metadata.type)
            return metadata.set;
    }

    return UINT32_MAX;
}

void CommandRecorder::BindPendingResources()
{
    VkResult result = VK_SUCCESS;

    if (mPendingResources.Empty())
        return;

    // TODO make this faster:
    //  - Create a cache/store for DS
    //  - Create DS when cache misses, reuse existing ones if possible
    //  - Bonus points for garbage collecting them after some time
    mTemporaryDescriptorSets.Resize(mPipelineState->mDescriptorSetLayouts.Size());

    VkDescriptorSetAllocateInfo allocInfo;
    VK_ZERO_MEMORY(allocInfo);
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pSetLayouts = mPipelineState->mDescriptorSetLayouts.Data();
    allocInfo.descriptorSetCount = mPipelineState->mDescriptorSetLayouts.Size();
    allocInfo.descriptorPool = gDevice->GetDescriptorPool();
    // TODO use temporary pool? might be easier to manage in case of fragmentation
    result = vkAllocateDescriptorSets(gDevice->GetDevice(), &allocInfo, mTemporaryDescriptorSets.Data());
    if (result != VK_SUCCESS)
    {
        NFE_LOG_WARNING("Failed to allocate temporary descriptor sets: %d (%s)", result, TranslateVkResultToString(result));
        return;
    }

    VkDescriptorBufferInfo bufferInfo;
    VK_ZERO_MEMORY(bufferInfo);
    VkDescriptorImageInfo imageInfo;
    VK_ZERO_MEMORY(imageInfo);
    Common::StaticArray<VkWriteDescriptorSet, VK_MAX_PENDING_RESOURCES> writeSets;

    for (uint32 i = 0; i < mPendingResources.Size(); ++i)
    {
        switch (mPendingResources[i].type)
        {
        case ShaderResourceType::UniformBuffer:
        case ShaderResourceType::StorageBuffer:
        {
            // TODO volatile buffers support
            Buffer* b = dynamic_cast<Buffer*>(mPendingResources[i].resource);
            VK_ZERO_MEMORY(bufferInfo);
            bufferInfo.buffer = b->mBuffer;
            bufferInfo.range = b->mBufferSize;
            bufferInfo.offset = 0;
            break;
        }
        case ShaderResourceType::SampledImage:
        case ShaderResourceType::StorageImage:
        {
            Texture* t = dynamic_cast<Texture*>(mPendingResources[i].resource);
            gDevice->GetLayoutTracker().EnsureLayout(mCommandBuffer, t->GetID(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            VK_ZERO_MEMORY(imageInfo);
            imageInfo.imageView = t->mImageView;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.sampler = nullptr;
            break;
        }
        case ShaderResourceType::Sampler:
        {
            Sampler* s = dynamic_cast<Sampler*>(mPendingResources[i].resource);
            VK_ZERO_MEMORY(imageInfo);
            imageInfo.sampler = s->mSampler;
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
        writeSet.descriptorType = mPipelineState->mDescriptorSetMetadata[setIdx].type;
        writeSet.dstSet = mTemporaryDescriptorSets[setIdx];
        writeSet.dstBinding = mPendingResources[i].slot;
        writeSet.pBufferInfo = (mPendingResources[i].type == ShaderResourceType::UniformBuffer ? &bufferInfo : nullptr);
        writeSet.pImageInfo =
            ((mPendingResources[i].type == ShaderResourceType::SampledImage) ||
            (mPendingResources[i].type == ShaderResourceType::Sampler) ?
            &imageInfo : nullptr);

        writeSets.EmplaceBack(writeSet);
    }

    vkUpdateDescriptorSets(gDevice->GetDevice(), writeSets.Size(), writeSets.Data(), 0, nullptr);

    vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            mPipelineState->mPipelineLayout, 0,
                            mTemporaryDescriptorSets.Size(), mTemporaryDescriptorSets.Data(),
                            0, nullptr);

    mPendingResources.Clear();
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
    mRebindDynamicBuffers = false;
    for (uint32 i = 0; i < VK_MAX_VOLATILE_BUFFERS; ++i)
    {
        mBoundVolatileBuffers[i] = nullptr;
        mBoundVolatileOffsets[i] = 0;
    }

    mRenderTarget = nullptr;
    ClearDescriptorSetBindings();

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
}

void CommandRecorder::CopyTexture(const TexturePtr& src, const TexturePtr& dest)
{
    NFE_UNUSED(src);
    NFE_UNUSED(dest);

    NFE_FATAL("Not implemented!");
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
        texSize = (texRegion->width - texRegion->x) *
                  (texRegion->height - texRegion->y) *
                  (texRegion->depth - texRegion->z);
        offsetLinear = (texRegion->width * texRegion->height) * texRegion->z +
                       (texRegion->width) * texRegion->y +
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
    CommandListPtr cl = gDevice->CreateCommandList(mQueueType, mCommandBuffer);
    mCommandBuffer = VK_NULL_HANDLE;
    mQueueType = CommandQueueType::Invalid;
    return cl;
}


// GRAPHICS PIPELINE METHODS //

void CommandRecorder::BindBuffer(ShaderType stage, uint32 slot, const BufferPtr& buffer, const BufferView& view)
{
    // TODO
    NFE_UNUSED(view);

    Buffer* b = dynamic_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(b != nullptr, "Invalid Buffer pointer");
    mPendingResources.EmplaceBack(b, stage, ShaderResourceType::UniformBuffer, slot);
}

void CommandRecorder::BindConstantBuffer(ShaderType stage, uint32 slot, const BufferPtr& buffer)
{
    Buffer* b = dynamic_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(b != nullptr, "Invalid Buffer pointer");

    if (b->mMode == ResourceAccessMode::Volatile)
    {

    }
    else
    {
        mPendingResources.EmplaceBack(b, stage, ShaderResourceType::UniformBuffer, slot);
    }
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
    NFE_UNUSED(stage);
    NFE_UNUSED(slot);
    NFE_UNUSED(buffer);
    NFE_UNUSED(view);

    //mPendingResources.EmplaceBack(s, stage, ShaderResourceType::StorageBuffer, slot);
}

void CommandRecorder::BindWritableTexture(ShaderType stage, uint32 slot, const TexturePtr& texture, const TextureView& view)
{
    NFE_UNUSED(stage);
    NFE_UNUSED(slot);
    NFE_UNUSED(texture);
    NFE_UNUSED(view);

    //mPendingResources.EmplaceBack(s, stage, ShaderResourceType::StorageImage, slot);
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
    mPipelineState = dynamic_cast<PipelineState*>(state.Get());
    NFE_ASSERT(mPipelineState != nullptr, "Incorrect pipeline state provided");

    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineState->mPipeline);
}

void CommandRecorder::SetComputePipelineState(const ComputePipelineStatePtr& state)
{
    ComputePipelineState* cps = dynamic_cast<ComputePipelineState*>(state.Get());
    NFE_ASSERT(cps != nullptr, "Invalid Compute Pipeline State provided");

    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, cps->mPipeline);
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
    /*if (mRebindDynamicBuffers)
    {
        RebindDynamicBuffers();
        mRebindDynamicBuffers = false;
    }*/

    BindPendingResources();
    EnsureInsideRenderPass();

    vkCmdDraw(mCommandBuffer, vertexNum, instancesNum, vertexOffset, instanceOffset);
}

void CommandRecorder::DrawIndexed(uint32 indexNum, uint32 instancesNum, uint32 indexOffset, int32 vertexOffset, uint32 instanceOffset)
{
    /*if (mRebindDynamicBuffers)
    {
        RebindDynamicBuffers();
        mRebindDynamicBuffers = false;
    }*/

    BindPendingResources();
    EnsureInsideRenderPass();

    vkCmdDrawIndexed(mCommandBuffer, indexNum, instancesNum, indexOffset, vertexOffset, instanceOffset);
}

void CommandRecorder::Dispatch(uint32 x, uint32 y, uint32 z)
{
    vkCmdDispatch(mCommandBuffer, x, y, z);
}

void CommandRecorder::DispatchIndirect(const BufferPtr& indirectArgBuffer, uint32 bufferOffset)
{
    NFE_UNUSED(indirectArgBuffer);
    NFE_UNUSED(bufferOffset);
}


// HINTS/RESOURCE QUEUE TRANSITIONS

void CommandRecorder::HintTargetCommandQueueType(const BufferPtr& resource, const CommandQueueType targetType)
{
    NFE_UNUSED(resource);
    NFE_UNUSED(targetType);
}

void CommandRecorder::HintTargetCommandQueueType(const TexturePtr& resource, const CommandQueueType targetType)
{
    NFE_UNUSED(resource);
    NFE_UNUSED(targetType);
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

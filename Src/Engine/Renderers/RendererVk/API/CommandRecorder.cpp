/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Command Recorder
 */

#include "PCH.hpp"

#include "CommandRecorder.hpp"
#include "CommandList.hpp"
#include "Device.hpp"
#include "ResourceBinding.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Backbuffer.hpp"
#include "PipelineState.hpp"
#include "ComputePipelineState.hpp"

#include "Internal/Commands.hpp"
#include "Internal/Translations.hpp"
#include "Internal/Debugger.hpp"

#include <Engine/Common/Math/Vec4fU.hpp>
#include <Engine/Common/Containers/UniquePtr.hpp>

#include <string.h>
#include <utility>


namespace NFE {
namespace Renderer {


CommandRecorder::CommandRecorder()
    : mCommandBuffer(VK_NULL_HANDLE)
    , mCommandBufferBeginInfo()
    , mCommandAllocator()
    , mState()
    , mCommands()
    , mCurrentBatch(0)
{
}

CommandRecorder::~CommandRecorder()
{
}


// PRIVATE METHODS //

bool CommandRecorder::WriteDynamicBuffer(Buffer* b, size_t offset, size_t size, const void* data)
{
    NFE_UNUSED(b);
    NFE_UNUSED(offset);
    NFE_UNUSED(size);
    NFE_UNUSED(data);
    // copy data to Ring Buffer
    /*uint32 sourceOffset = gDevice->GetRingBuffer()->Write(data, static_cast<uint32>(size));

    if (sourceOffset == UINT32_MAX)
    {
        NFE_LOG_ERROR("Failed to write temporary data to Ring Ruffer - the Ring Buffer is full");
        return false;
    }

    // CopyBuffer has to be call outside a Render Pass
    if (mRenderTarget)
        vkCmdEndRenderPass(mCommandBuffer);

    VkBufferCopy region;
    VK_ZERO_MEMORY(region);
    region.size = size;
    region.srcOffset = static_cast<VkDeviceSize>(sourceOffset);
    region.dstOffset = static_cast<VkDeviceSize>(offset);
    vkCmdCopyBuffer(mCommandBuffer, gDevice->GetRingBuffer()->GetVkBuffer(), b->mBuffer, 1, &region);

    // Restore previously-active RenderPass after copy to buffer is done
    if (mRenderTarget)
    {
        VkRenderPassBeginInfo rpBeginInfo;
        VK_ZERO_MEMORY(rpBeginInfo);
        rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBeginInfo.renderPass = mRenderTarget->mRenderPass;
        rpBeginInfo.framebuffer = mRenderTarget->mFramebuffer;
        rpBeginInfo.renderArea.offset = { 0, 0 };
        rpBeginInfo.renderArea.extent = { static_cast<uint32>(mRenderTarget->mWidth),
                                          static_cast<uint32>(mRenderTarget->mHeight) };
        vkCmdBeginRenderPass(mCommandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }
    */
    return false;
}

bool CommandRecorder::WriteVolatileBuffer(Buffer* b, size_t size, const void* data)
{
    NFE_UNUSED(b);
    NFE_UNUSED(size);
    NFE_UNUSED(data);

    /*
    b->mVolatileDataOffset = gDevice->GetRingBuffer()->Write(data, static_cast<uint32>(size));
    NFE_ASSERT(b->mVolatileDataOffset != UINT32_MAX, "Failed to write data to Ring Ruffer - the Ring Buffer is full");

    if (b->mType == BufferType::Constant)
    {
        for (uint32 i = 0; i < NFE_VK_MAX_VOLATILE_BUFFERS; ++i)
        {
            if (b == mBoundVolatileBuffers[i])
            {
                mBoundVolatileOffsets[i] = b->mVolatileDataOffset;
                mRebindDynamicBuffers = true;
                break;
            }
        }
    }*/

    return false;
}

void CommandRecorder::SwitchToNewBatch()
{
    mCommands.EmplaceBack(mCommandAllocator);
    mCurrentBatch++;
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

bool CommandRecorder::Begin()
{
    mState.Clear();
    mCommandAllocator.Clear();

    mCommands.EmplaceBack(mCommandAllocator);
    mCurrentBatch = 0;

    return true;
}

void CommandRecorder::CopyTexture(const TexturePtr& src, const TexturePtr& dest)
{
    Texture* s = dynamic_cast<Texture*>(src.Get());
    Texture* d = dynamic_cast<Texture*>(dest.Get());

    NFE_ASSERT(s != nullptr, "Invalid source texture provided");
    NFE_ASSERT(d != nullptr, "Invalid destination texture provided");

    // TODO more specific region copy - use srcOffset/dstOffset
    VkImageCopy copyRegion;
    VK_ZERO_MEMORY(copyRegion);
    copyRegion.srcSubresource.aspectMask = s->mImageSubresRange.aspectMask;
    copyRegion.srcSubresource.baseArrayLayer = s->mImageSubresRange.baseArrayLayer;
    copyRegion.srcSubresource.layerCount = s->mImageSubresRange.layerCount;
    copyRegion.dstSubresource.aspectMask = d->mImageSubresRange.aspectMask;
    copyRegion.dstSubresource.baseArrayLayer = d->mImageSubresRange.baseArrayLayer;
    copyRegion.dstSubresource.layerCount = d->mImageSubresRange.layerCount;
    copyRegion.extent.width = s->mWidth;
    copyRegion.extent.height = s->mHeight;
    copyRegion.extent.depth = 1;
    mCommands[mCurrentBatch].Record<CopyTextureCommand>(s->mImage, d->mImage, copyRegion);
}

void CommandRecorder::CopyTexture(const TexturePtr& src, const BackbufferPtr& dest)
{
    Texture* s = dynamic_cast<Texture*>(src.Get());
    Backbuffer* d = dynamic_cast<Backbuffer*>(dest.Get());

    NFE_ASSERT(s != nullptr, "Invalid source texture provided");
    NFE_ASSERT(d != nullptr, "Invalid destination backbuffer provided");

    // TEMPORARY - this should be injected on Finish()
    mCommands[mCurrentBatch].Record<EndRenderPassCommand>();

    VkImageMemoryBarrier imageBarriers[2];
    VK_ZERO_MEMORY(imageBarriers[0]);
    imageBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarriers[0].srcAccessMask = 0;
    imageBarriers[0].dstAccessMask = 0;
    imageBarriers[0].oldLayout = s->mImageLayout;
    imageBarriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imageBarriers[0].image = s->mImage;
    imageBarriers[0].subresourceRange = s->mImageSubresRange;
    VK_ZERO_MEMORY(imageBarriers[1]);
    imageBarriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarriers[1].srcAccessMask = 0;
    imageBarriers[1].dstAccessMask = 0;
    imageBarriers[1].oldLayout = d->mImageExtraDatas[d->mCurrentImage].layout;
    imageBarriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarriers[1].image = d->mImages[d->mCurrentImage];
    imageBarriers[1].subresourceRange = d->mImageSubresRange;
    mCommands[mCurrentBatch].Record<PipelineBarrierCommand>(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                            2, imageBarriers);

    if ((s->mFormat == d->mFormat) && (s->mWidth == d->mWidth) && (s->mHeight == d->mHeight))
    {
        // do a raw copy, which doesn't do format conversion/scaling and is faster than blitting
        VkImageCopy copyRegion;
        VK_ZERO_MEMORY(copyRegion);
        copyRegion.srcSubresource.aspectMask = s->mImageSubresRange.aspectMask;
        copyRegion.srcSubresource.baseArrayLayer = s->mImageSubresRange.baseArrayLayer;
        copyRegion.srcSubresource.layerCount = s->mImageSubresRange.layerCount;
        copyRegion.dstSubresource.aspectMask = d->mImageSubresRange.aspectMask;
        copyRegion.dstSubresource.baseArrayLayer = d->mImageSubresRange.baseArrayLayer;
        copyRegion.dstSubresource.layerCount = d->mImageSubresRange.layerCount;
        copyRegion.extent.width = s->mWidth;
        copyRegion.extent.height = s->mHeight;
        copyRegion.extent.depth = 1;
        mCommands[mCurrentBatch].Record<CopyTextureCommand>(s->mImage, d->mImages[d->mCurrentImage], copyRegion);
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
        blitRegion.dstSubresource.aspectMask = d->mImageSubresRange.aspectMask;
        blitRegion.dstSubresource.baseArrayLayer = d->mImageSubresRange.baseArrayLayer;
        blitRegion.dstSubresource.layerCount = d->mImageSubresRange.layerCount;
        // source offsets - define region to copy from
        blitRegion.srcOffsets[1].x = s->mWidth;
        blitRegion.srcOffsets[1].y = s->mHeight;
        blitRegion.srcOffsets[1].z = 1;
        // destination offsets - define region where to copy to
        blitRegion.dstOffsets[1].x = d->mWidth;
        blitRegion.dstOffsets[1].y = d->mHeight;
        blitRegion.dstOffsets[1].z = 1;
        mCommands[mCurrentBatch].Record<BlitTextureCommand>(s->mImage, d->mImages[d->mCurrentImage], blitRegion);
    }

    SwitchToNewBatch();

    // TODO Move to ResourceState
    imageBarriers[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imageBarriers[0].newLayout = s->mImageLayoutDefault;
    imageBarriers[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarriers[1].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    mCommands[mCurrentBatch].Record<PipelineBarrierCommand>(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                            2, imageBarriers);
}

bool CommandRecorder::WriteBuffer(const BufferPtr& buffer, size_t offset, size_t size, const void* data)
{
    Buffer* b = dynamic_cast<Buffer*>(buffer.Get());
    if (b == nullptr)
    {
        NFE_LOG_ERROR("Invalid buffer pointer provided");
        return false;
    }

    if (b->mMode == BufferMode::Dynamic)
    {
        if (static_cast<VkDeviceSize>(size) > b->mBufferSize)
        {
            NFE_LOG_ERROR("Requested to write more than provided buffer can handle (%d vs buffer's %d)",
                      size, b->mBufferSize);
            return false;
        }

        return WriteDynamicBuffer(b, offset, size, data);
    }
    else if (b->mMode == BufferMode::Volatile)
    {
        return WriteVolatileBuffer(b, size, data);
    }
    else
    {
        NFE_LOG_ERROR("Provided buffer does not have a CPU-writable mode");
        return false;
    }
}

CommandListPtr CommandRecorder::Finish()
{
    // TODO before actually recording commands, drop extra commands:
    //  - Pipeline barriers from ResourceState
    //  - EndRenderPass and BeginRenderPass if there are necessary barriers in-between batches
    //  - EndRenderPass at last batch

    VkCommandBuffer commandBuffer;
    uint32 cbID = 0;
    if (!gDevice->GetAvailableCommandBuffer(commandBuffer, cbID))
    {
        NFE_LOG_ERROR("No more available command buffers to record to!");
        return nullptr;
    }

    vkBeginCommandBuffer(commandBuffer, &mCommandBufferBeginInfo);

    for (uint32 i = 0; i < mCommands.Size(); ++i)
        mCommands[i].Commit(commandBuffer, mState);

    VkResult result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS)
    {
        NFE_LOG_ERROR("Error during Constant Buffer recording: %d (%s)",
                  result, TranslateVkResultToString(result));
        return nullptr;
    }

    mCommands.Clear();

    return Common::MakeUniquePtr<CommandList>(cbID);
}


// GRAPHICS PIPELINE METHODS //

void CommandRecorder::BindResources(uint32 slot, const ResourceBindingInstancePtr& bindingSetInstance)
{
    NFE_UNUSED(slot);

    ResourceBindingInstance* rbi = dynamic_cast<ResourceBindingInstance*>(bindingSetInstance.Get());
    if (rbi == nullptr)
        return; // there is no "unbind" of descriptor sets in Vulkan
    /* TODO
    for (auto& r: rbi->mWrittenResources)
    {
        if ((r != nullptr) && (r->GetType() == ShaderResourceType::Texture))
        {
            Texture* t = dynamic_cast<Texture*>(r);
            t->Transition(mCommandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }

    vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            mResourceBindingLayout->mPipelineLayout, rbi->mSet->mSetSlot, 1,
                            &rbi->mDescriptorSet, 0, nullptr);*/
}

void CommandRecorder::BindVolatileCBuffer(uint32 slot, const BufferPtr& buffer)
{
    Buffer* b = dynamic_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(b != nullptr, "Invalid volatile buffer provided");
    NFE_ASSERT(b->mMode == BufferMode::Volatile, "Buffer is not a Volatile Buffer");
    NFE_ASSERT(b->mType == BufferType::Constant, "Buffer is not a Constant Buffer");
    NFE_ASSERT(slot < NFE_VK_MAX_VOLATILE_BUFFERS, "Binding to slot %d impossible (max available slots 0-7).", slot);

    // TODO handle Volatile Buffers differently - see BindComputeVolatileCBuffer
    /*mBoundVolatileBuffers[slot] = b;
    mRebindDynamicBuffers = true;

    if (b->mVolatileDataOffset != UINT32_MAX)
    {
        mBoundVolatileOffsets[slot] = b->mVolatileDataOffset;
    }*/
}

void CommandRecorder::Clear(uint32 flags, uint32 numTargets, const uint32* slots, const Math::Vec4fU* colors, float depthValue, uint8 stencilValue)
{
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

    mCommands[mCurrentBatch].Record<ClearCommand>(clearAtts, clearAttsNum);
}

void CommandRecorder::SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format)
{
    Buffer* ib = dynamic_cast<Buffer*>(indexBuffer.Get());
    NFE_ASSERT(ib != nullptr, "Incorrect Index Buffer provided");

    mCommands[mCurrentBatch].Record<BindIndexBufferCommand>(ib->mBuffer, 0, TranslateIndexBufferFormatToVkIndexType(format));
}

void CommandRecorder::SetRenderTarget(const RenderTargetPtr& renderTarget)
{
    RenderTarget* rt = dynamic_cast<RenderTarget*>(renderTarget.Get());
    NFE_ASSERT(rt != nullptr, "Invalid Render Target provided");

    VkRenderPassBeginInfo rpBeginInfo;
    VK_ZERO_MEMORY(rpBeginInfo);
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = rt->mRenderPass;
    rpBeginInfo.framebuffer = rt->mFramebuffer;
    rpBeginInfo.renderArea.offset = { 0, 0 };
    rpBeginInfo.renderArea.extent = { static_cast<uint32>(rt->mWidth),
                                      static_cast<uint32>(rt->mHeight) };
    mCommands[mCurrentBatch].Record<BeginRenderPassCommand>(rpBeginInfo);
}

void CommandRecorder::SetResourceBindingLayout(const ResourceBindingLayoutPtr& layout)
{
    ResourceBindingLayout* rbl = dynamic_cast<ResourceBindingLayout*>(layout.Get());
    NFE_ASSERT(rbl != nullptr, "Incorrect binding layout provided");

    mCommands[mCurrentBatch].Record<BindPipelineLayoutCommand>(rbl->mPipelineLayout);
}

void CommandRecorder::SetPipelineState(const PipelineStatePtr& state)
{
    PipelineState* ps = dynamic_cast<PipelineState*>(state.Get());
    NFE_ASSERT(ps != nullptr, "Incorrect pipeline state provided");

    mCommands[mCurrentBatch].Record<BindPipelineCommand>(VK_PIPELINE_BIND_POINT_GRAPHICS, ps->mPipeline);
}

void CommandRecorder::SetScissors(int32 left, int32 top, int32 right, int32 bottom)
{
    VkRect2D scissor;
    scissor.offset.x = left;
    scissor.offset.y = top;
    scissor.extent.width = right - left;
    scissor.extent.height = bottom - top;
    mCommands[mCurrentBatch].Record<SetScissorCommand>(scissor);
}

void CommandRecorder::SetStencilRef(unsigned char ref)
{
    mCommands[mCurrentBatch].Record<SetStencilRefCommand>(ref);
}

void CommandRecorder::SetVertexBuffers(uint32 num, const BufferPtr* vertexBuffers, uint32* strides, uint32* offsets)
{
    NFE_UNUSED(strides);

    VkBuffer buffers[NFE_VK_MAX_VERTEX_BUFFERS];
    VkDeviceSize offs[NFE_VK_MAX_VERTEX_BUFFERS];

    for (uint32 i = 0; i < num; ++i)
    {
        Buffer* buf = dynamic_cast<Buffer*>(vertexBuffers[i].Get());
        NFE_ASSERT(buf != nullptr, "Incorrect buffer provided at slot %d", i);

        if (buf->mMode == BufferMode::Volatile)
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
    mCommands[mCurrentBatch].Record<BindVertexBuffersCommand>(0, num, buffers, offs);
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
    mCommands[mCurrentBatch].Record<SetViewportCommand>(viewport);
}

void CommandRecorder::Draw(uint32 vertexNum, uint32 instancesNum, uint32 vertexOffset, uint32 instanceOffset)
{
    mCommands[mCurrentBatch].Record<DrawCommand>(vertexNum, instancesNum, vertexOffset, instanceOffset);
    SwitchToNewBatch();
}

void CommandRecorder::DrawIndexed(uint32 indexNum, uint32 instancesNum, uint32 indexOffset, int32 vertexOffset, uint32 instanceOffset)
{
    mCommands[mCurrentBatch].Record<DrawIndexedCommand>(indexNum, instancesNum, indexOffset, vertexOffset, instanceOffset);
    SwitchToNewBatch();
}


// COMPUTE PIPELINE METHODS

void CommandRecorder::BindComputeResources(uint32 slot, const ResourceBindingInstancePtr& bindingSetInstance)
{
    NFE_UNUSED(slot);

    ResourceBindingInstance* rbi = dynamic_cast<ResourceBindingInstance*>(bindingSetInstance.Get());
    if (rbi == nullptr)
        return; // there is no "unbind" of descriptor sets in Vulkan

    // TODO ResourceState has to transition the layout
    mCommands[mCurrentBatch].Record<BindDescriptorSetCommand>(VK_PIPELINE_BIND_POINT_COMPUTE,
                                                              static_cast<uint32>(rbi->mSet->mSetSlot),
                                                              rbi->mDescriptorSet);
}

void CommandRecorder::BindComputeVolatileCBuffer(uint32 slot, const BufferPtr& buffer)
{
    NFE_UNUSED(slot);

    Buffer* b = dynamic_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(b != nullptr, "Invalid volatile CBuffer provided");
    NFE_ASSERT(b->mMode == BufferMode::Volatile, "Buffer is not a Volatile Buffer");
    NFE_ASSERT(b->mType == BufferType::Constant, "Buffer is not a Constant Buffer");
    NFE_ASSERT(slot < NFE_VK_MAX_VOLATILE_BUFFERS, "Binding to slot %d impossible (max available slots 0-7).", slot);

    // TODO handle Volatile Buffers differently:
    // - Create (maybe a part of Resource State?) a volatile buffer entry
    // - WriteVolatileBuffer should create a connection Buffer -> VolatileBufferWrite in ResourceState
    // - New WriteVolatileBuffer for the same Buffer overwrites offset on that entry
    // - Binding here refers to ResourceState and records binding and RingBuffer offset
    // - Destroying the Volatile Buffer clears the entry
    NFE_ASSERT(0, "Not yet implemented");
}

void CommandRecorder::SetComputePipelineState(const ComputePipelineStatePtr& state)
{
    ComputePipelineState* cps = dynamic_cast<ComputePipelineState*>(state.Get());
    NFE_ASSERT(cps != nullptr, "Invalid Compute Pipeline State provided");

    mCommands[mCurrentBatch].Record<BindPipelineCommand>(VK_PIPELINE_BIND_POINT_COMPUTE, cps->mPipeline);
}

void CommandRecorder::SetComputeResourceBindingLayout(const ResourceBindingLayoutPtr& layout)
{
    ResourceBindingLayout* rbl = dynamic_cast<ResourceBindingLayout*>(layout.Get());
    NFE_ASSERT(rbl != nullptr, "Invalid Compute Resource Binding Layout provided");

    mCommands[mCurrentBatch].Record<BindPipelineLayoutCommand>(rbl->mPipelineLayout);
}

void CommandRecorder::Dispatch(uint32 x, uint32 y, uint32 z)
{
    mCommands[mCurrentBatch].Record<DispatchCommand>(x, y, z);
    SwitchToNewBatch();
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

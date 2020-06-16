/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Command Recorder
 */

#include "PCH.hpp"

#include "CommandRecorder.hpp"
#include "Device.hpp"
#include "ResourceBinding.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Backbuffer.hpp"

#include "Internal/Translations.hpp"

#include "Engine/Common/Math/Vec4fU.hpp"

#include <string.h>


namespace NFE {
namespace Renderer {

CommandRecorder::CommandRecorder()
    : mCommandBuffer(VK_NULL_HANDLE)
    , mCommandBufferBeginInfo()
    , mRenderTarget(nullptr)
    , mActiveRenderPass(false)
    , mResourceBindingLayout(nullptr)
    , mBoundVolatileBuffers()
    , mBoundVolatileOffsets()
    , mRebindDynamicBuffers(false)
{
}

CommandRecorder::~CommandRecorder()
{
}

bool CommandRecorder::Init()
{
    VK_ZERO_MEMORY(mCommandBufferBeginInfo);
    mCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    mCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    NFE_LOG_INFO("Command Buffer initialized successfully.");
    return true;
}

bool CommandRecorder::Begin()
{
    mRebindDynamicBuffers = false;
    for (uint32 i = 0; i < VK_MAX_VOLATILE_BUFFERS; ++i)
    {
        mBoundVolatileBuffers[i] = nullptr;
        mBoundVolatileOffsets[i] = 0;
    }

    mRenderTarget = nullptr;

    // acquire command buffer from pool in Device
    mCommandBuffer = gDevice->GetAvailableCommandBuffer();
    vkBeginCommandBuffer(mCommandBuffer, &mCommandBufferBeginInfo);

    return true;
}

void CommandRecorder::SetVertexBuffers(uint32 num, const BufferPtr* vertexBuffers, uint32* strides, uint32* offsets)
{
    NFE_UNUSED(strides);

    const uint32 maxBuffers = 4;
    VkBuffer buffers[maxBuffers];
    VkDeviceSize offs[maxBuffers];

    for (uint32 i = 0; i < num; ++i)
    {
        Buffer* buf = dynamic_cast<Buffer*>(vertexBuffers[i].Get());
        if (!buf)
        {
            NFE_LOG_ERROR("Incorrect buffer provided at slot %d", i);
            return;
        }

        buffers[i] = buf->mBuffer;
        offs[i] = static_cast<VkDeviceSize>(offsets[i]);
    }

    // TODO assumes start slot 0
    vkCmdBindVertexBuffers(mCommandBuffer, 0, num, buffers, offs);
}

void CommandRecorder::SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format)
{
    Buffer* ib = dynamic_cast<Buffer*>(indexBuffer.Get());
    if (ib == nullptr)
    {
        NFE_LOG_ERROR("Incorrect Index Buffer provided");
        return;
    }

    vkCmdBindIndexBuffer(mCommandBuffer, ib->mBuffer, 0, TranslateIndexBufferFormatToVkIndexType(format));
}

void CommandRecorder::BindResources(uint32 slot, const ResourceBindingInstancePtr& bindingSetInstance)
{
    NFE_UNUSED(slot);

    ResourceBindingInstance* rbi = dynamic_cast<ResourceBindingInstance*>(bindingSetInstance.Get());
    if (rbi == nullptr)
    {
        NFE_LOG_ERROR("Incorrect resource binding instance provided");
        return;
    }

    vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            mResourceBindingLayout->mPipelineLayout, rbi->mSet->mSetSlot, 1,
                            &rbi->mDescriptorSet, 0, nullptr);
}

void CommandRecorder::BindVolatileCBuffer(uint32 slot, const BufferPtr& buffer)
{
    Buffer* b = dynamic_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(b != nullptr, "Invalid volatile buffer provided");
    NFE_ASSERT(b->mMode == BufferMode::Volatile, "Buffer with invalid mode provided");
    NFE_ASSERT(slot < VK_MAX_VOLATILE_BUFFERS, "Binding to slot %d impossible (max available slots 0-7).", slot);

    mBoundVolatileBuffers[slot] = b;
    mRebindDynamicBuffers = true;
}

void CommandRecorder::SetResourceBindingLayout(const ResourceBindingLayoutPtr& layout)
{
    mResourceBindingLayout = dynamic_cast<ResourceBindingLayout*>(layout.Get());
    NFE_ASSERT(mResourceBindingLayout != nullptr, "Incorrect binding layout provided");
}

void CommandRecorder::SetRenderTarget(const RenderTargetPtr& renderTarget)
{
    if (mRenderTarget)
    {
        // there is a previous render pass active, end it
        vkCmdEndRenderPass(mCommandBuffer);
    }

    mRenderTarget = dynamic_cast<RenderTarget*>(renderTarget.Get());
    NFE_ASSERT(mRenderTarget != nullptr, "Invalid Render Target pointer provided");

    mRenderTarget->TransitionColorAttachments(mCommandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    mRenderTarget->TransitionDSAttachment(mCommandBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

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

void CommandRecorder::SetPipelineState(const PipelineStatePtr& state)
{
    PipelineState* ps = dynamic_cast<PipelineState*>(state.Get());
    NFE_ASSERT(ps != nullptr, "Incorrect pipeline state provided");

    // TODO support compute bind point
    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ps->mPipeline);
}

void CommandRecorder::SetStencilRef(unsigned char ref)
{
    NFE_UNUSED(ref);
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

void CommandRecorder::SetScissors(int32 left, int32 top, int32 right, int32 bottom)
{
    VkRect2D scissor;
    scissor.offset.x = left;
    scissor.offset.y = top;
    scissor.extent.width = right - left;
    scissor.extent.height = bottom - top;
    vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
}

void* CommandRecorder::MapBuffer(const BufferPtr& buffer, MapType type)
{
    NFE_UNUSED(buffer);
    NFE_UNUSED(type);
    return nullptr;
}

void CommandRecorder::UnmapBuffer(const BufferPtr& buffer)
{
    NFE_UNUSED(buffer);
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

    if (mRenderTarget)
        vkCmdEndRenderPass(mCommandBuffer);

    VkBufferCopy region;
    VK_ZERO_MEMORY(region);
    region.size = size;
    region.srcOffset = static_cast<VkDeviceSize>(sourceOffset);
    region.dstOffset = static_cast<VkDeviceSize>(offset);
    vkCmdCopyBuffer(mCommandBuffer, gDevice->GetRingBuffer()->GetVkBuffer(), b->mBuffer, 1, &region);

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

    return true;
}

bool CommandRecorder::WriteVolatileBuffer(Buffer* b, size_t size, const void* data)
{
    NFE_UNUSED(b);

    uint32 writeHead = gDevice->GetRingBuffer()->Write(data, static_cast<uint32>(size));
    NFE_ASSERT(writeHead != UINT32_MAX, "Failed to write data to Ring Ruffer - the Ring Buffer is full");

    for (uint32 i = 0; i < VK_MAX_VOLATILE_BUFFERS; ++i)
    {
        if (b == mBoundVolatileBuffers[i])
        {
            mBoundVolatileOffsets[i] = writeHead;
            mRebindDynamicBuffers = true;
            break;
        }
    }

    return true;
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

    if (mRenderTarget)
    {
        vkCmdEndRenderPass(mCommandBuffer);
        mRenderTarget = nullptr;
    }

    s->Transition(mCommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    d->Transition(mCommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

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
    vkCmdCopyImage(mCommandBuffer,
                   s->mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   d->mImages[d->mCurrentImage], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &copyRegion);

    d->Transition(mCommandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

void CommandRecorder::Clear(uint32 flags, uint32 numTargets, const uint32* slots, const Math::Vec4fU* colors, float depthValue, uint8 stencilValue)
{
    if (!mRenderTarget)
        return;

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

void CommandRecorder::RebindDynamicBuffers() const
{
    uint32 offsets[VK_MAX_VOLATILE_BUFFERS];
    uint32 offsetCount = 0;
    for (uint32 i = 0; i < VK_MAX_VOLATILE_BUFFERS; ++i)
    {
        if (mBoundVolatileBuffers[i] != nullptr)
        {
            offsets[offsetCount++] = mBoundVolatileOffsets[i];
        }
    }

    if (offsetCount > 0)
    {
        vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mResourceBindingLayout->mPipelineLayout,
                                mResourceBindingLayout->mVolatileBufferSetSlot, 1, &mResourceBindingLayout->mVolatileBufferSet,
                                offsetCount, offsets);
    }
}

void CommandRecorder::Draw(uint32 vertexNum, uint32 instancesNum, uint32 vertexOffset, uint32 instanceOffset)
{
    if (mRebindDynamicBuffers)
    {
        RebindDynamicBuffers();
        mRebindDynamicBuffers = false;
    }

    vkCmdDraw(mCommandBuffer, vertexNum, instancesNum, vertexOffset, instanceOffset);
}

void CommandRecorder::DrawIndexed(uint32 indexNum, uint32 instancesNum, uint32 indexOffset, int32 vertexOffset, uint32 instanceOffset)
{
    if (mRebindDynamicBuffers)
    {
        RebindDynamicBuffers();
        mRebindDynamicBuffers = false;
    }

    vkCmdDrawIndexed(mCommandBuffer, indexNum, instancesNum, indexOffset, vertexOffset, instanceOffset);
}

void CommandRecorder::BindComputeResources(uint32 slot, const ResourceBindingInstancePtr& bindingSetInstance)
{
    NFE_UNUSED(slot);
    NFE_UNUSED(bindingSetInstance);
}

void CommandRecorder::BindComputeVolatileCBuffer(uint32 slot, const BufferPtr& buffer)
{
    NFE_UNUSED(slot);
    NFE_UNUSED(buffer);
}

void CommandRecorder::SetComputeResourceBindingLayout(const ResourceBindingLayoutPtr& layout)
{
    NFE_UNUSED(layout);
}

void CommandRecorder::SetComputePipelineState(const ComputePipelineStatePtr& state)
{
    NFE_UNUSED(state);
}

void CommandRecorder::Dispatch(uint32 x, uint32 y, uint32 z)
{
    NFE_UNUSED(x);
    NFE_UNUSED(y);
    NFE_UNUSED(z);
}

CommandListID CommandRecorder::Finish()
{
    if (mRenderTarget)
    {
        vkCmdEndRenderPass(mCommandBuffer);
    }

    VkResult result = vkEndCommandBuffer(mCommandBuffer);
    if (result != VK_SUCCESS)
    {
        NFE_LOG_ERROR("Error during Constant Buffer recording: %d (%s)",
                  result, TranslateVkResultToString(result));
        return 0;
    }

    return gDevice->GetCurrentCommandBuffer() + 1;
}

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

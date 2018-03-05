/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Command Recorder
 */

#include "PCH.hpp"

#include "CommandRecorder.hpp"
#include "Device.hpp"
#include "ResourceBinding.hpp"

#include "Internal/Translations.hpp"

#include "nfCommon/Math/Vector4.hpp"

#include <string.h>


namespace NFE {
namespace Renderer {

CommandRecorder::CommandRecorder()
    : mCommandBuffer(VK_NULL_HANDLE)
    , mRenderTarget(nullptr)
    , mActiveRenderPass(false)
    , mResourceBindingLayout(nullptr)
{
}

CommandRecorder::~CommandRecorder()
{
}

bool CommandRecorder::Init(Common::SharedPtr<Device>& device)
{
    mDevicePtr = device;

    VK_ZERO_MEMORY(mCommandBufferBeginInfo);
    mCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    mCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    NFE_LOG_INFO("Command Buffer initialized successfully.");
    return true;
}

bool CommandRecorder::Begin()
{
    for (uint32 i = 0; i < VK_MAX_VOLATILE_BUFFERS; ++i)
        mBoundVolatileBuffers[i] = nullptr;

    mRenderTarget = nullptr;

    // acquire command buffer from pool in Device
    mCommandBuffer = mDevicePtr->GetAvailableCommandBuffer();
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
                            &rbi->mSet->mDescriptorSet, 0, nullptr);
}

void CommandRecorder::BindVolatileCBuffer(uint32 slot, const BufferPtr& buffer)
{
    Buffer* b = dynamic_cast<Buffer*>(buffer.Get());
    if (b == nullptr)
    {
        NFE_LOG_ERROR("Invalid volatile buffer provided");
        return;
    }

    if (b->mMode != BufferMode::Volatile)
    {
        NFE_LOG_ERROR("Buffer with invalid mode provided");
        return;
    }

    if (slot >= VK_MAX_VOLATILE_BUFFERS)
    {
        NFE_LOG_ERROR("Binding to slot %d impossible (max available slots 0-7).", slot);
        return;
    }

    if (b != mBoundVolatileBuffers[slot])
    {
        mBoundVolatileBuffers[slot] = b;
    }
}

void CommandRecorder::SetResourceBindingLayout(const ResourceBindingLayoutPtr& layout)
{
    mResourceBindingLayout = dynamic_cast<ResourceBindingLayout*>(layout.Get());
    if (!mResourceBindingLayout)
        NFE_LOG_ERROR("Incorrect binding layout provided");
}

void CommandRecorder::SetRenderTarget(const RenderTargetPtr& renderTarget)
{
    if (mRenderTarget)
    {
        // there is a previous render pass active, end it
        vkCmdEndRenderPass(mCommandBuffer);
    }

    mRenderTarget = dynamic_cast<RenderTarget*>(renderTarget.Get());
    if (!mRenderTarget)
    {
        NFE_LOG_ERROR("Incorrect Render Target pointer.");
        return;
    }

    for (uint32 i = 0; i < mRenderTarget->mTex.Size(); ++i)
    {
        mRenderTarget->mTex[i]->TransitionCurrentTexture(mCommandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    if (mRenderTarget->mDepthTex)
        mRenderTarget->mDepthTex->TransitionCurrentTexture(mCommandBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    VkRenderPassBeginInfo rpBeginInfo;
    VK_ZERO_MEMORY(rpBeginInfo);
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = mRenderTarget->mRenderPass;
    rpBeginInfo.framebuffer = mRenderTarget->GetCurrentFramebuffer();
    rpBeginInfo.renderArea.offset = { 0, 0 };
    rpBeginInfo.renderArea.extent = { static_cast<uint32>(mRenderTarget->mWidth),
                                      static_cast<uint32>(mRenderTarget->mHeight) };
    vkCmdBeginRenderPass(mCommandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandRecorder::SetPipelineState(const PipelineStatePtr& state)
{
    PipelineState* ps = dynamic_cast<PipelineState*>(state.Get());
    if (ps == nullptr)
    {
        NFE_LOG_ERROR("Incorrect pipeline state provided");
        return;
    }

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
    viewport.y = top;
    viewport.width = width;
    viewport.height = height;
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
    uint32 sourceOffset = mDevicePtr->GetRingBuffer()->Write(data, static_cast<uint32>(size));

    if (sourceOffset == std::numeric_limits<uint32>::max())
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
    vkCmdCopyBuffer(mCommandBuffer, mDevicePtr->GetRingBuffer()->GetVkBuffer(), b->mBuffer, 1, &region);

    if (mRenderTarget)
    {
        VkRenderPassBeginInfo rpBeginInfo;
        VK_ZERO_MEMORY(rpBeginInfo);
        rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBeginInfo.renderPass = mRenderTarget->mRenderPass;
        rpBeginInfo.framebuffer = mRenderTarget->GetCurrentFramebuffer();
        rpBeginInfo.renderArea.offset = { 0, 0 };
        rpBeginInfo.renderArea.extent = { static_cast<uint32>(mRenderTarget->mWidth),
                                          static_cast<uint32>(mRenderTarget->mHeight) };
        vkCmdBeginRenderPass(mCommandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    return true;
}

bool CommandRecorder::WriteVolatileBuffer(Buffer* b, size_t size, const void* data)
{
    uint32 writeHead = mDevicePtr->GetRingBuffer()->Write(data, static_cast<uint32>(size));
    if (writeHead == std::numeric_limits<uint32>::max())
    {
        NFE_LOG_ERROR("Failed to write data to Ring Ruffer - the Ring Buffer is full");
        return false;
    }

    for (uint32 i = 0; i < VK_MAX_VOLATILE_BUFFERS; ++i)
    {
        if (b == mBoundVolatileBuffers[i])
        {
            vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mResourceBindingLayout->mPipelineLayout,
                                    mResourceBindingLayout->mVolatileSetSlot, 1, &mResourceBindingLayout->mVolatileBufferSet,
                                    1, &writeHead);
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
            NFE_LOG_ERROR("Requested to write more than provided buffer can handle (%d vs buffer's $d)",
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
}

void CommandRecorder::Clear(uint32 flags, uint32 numTargets, const uint32* slots, const Math::Float4* colors, float depthValue, uint8 stencilValue)
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


void CommandRecorder::Draw(uint32 vertexNum, uint32 instancesNum, uint32 vertexOffset, uint32 instanceOffset)
{
    vkCmdDraw(mCommandBuffer, vertexNum, instancesNum, vertexOffset, instanceOffset);
}

void CommandRecorder::DrawIndexed(uint32 indexNum, uint32 instancesNum, uint32 indexOffset, int32 vertexOffset, uint32 instanceOffset)
{
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

    return mDevicePtr->GetCurrentCommandBuffer() + 1;
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

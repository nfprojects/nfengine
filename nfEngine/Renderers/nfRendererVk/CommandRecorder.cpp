/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Command Recorder
 */

#include "PCH.hpp"

#include "CommandRecorder.hpp"
#include "Device.hpp"
#include "Translations.hpp"
#include "ResourceBinding.hpp"

#include "nfCommon/Math/Vector.hpp"

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

bool CommandRecorder::Init()
{
    VK_ZERO_MEMORY(mCommandBufferBeginInfo);
    mCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    mCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    LOG_INFO("Command Buffer initialized successfully.");
    return true;
}

bool CommandRecorder::Begin()
{
    for (uint32 i = 0; i < VK_MAX_VOLATILE_BUFFERS; ++i)
        mBoundVolatileBuffers[i] = nullptr;

    mRenderTarget = nullptr;

    // acquire command buffer from pool in Device
    mCommandBuffer = gDevice->GetAvailableCommandBuffer();
    vkBeginCommandBuffer(mCommandBuffer, &mCommandBufferBeginInfo);

    return true;
}

void CommandRecorder::SetVertexBuffers(int num, const BufferPtr* vertexBuffers, int* strides, int* offsets)
{
    UNUSED(strides);

    const int maxBuffers = 4;
    VkBuffer buffers[maxBuffers];
    VkDeviceSize offs[maxBuffers];

    for (int i = 0; i < num; ++i)
    {
        Buffer* buf = dynamic_cast<Buffer*>(vertexBuffers[i].get());
        if (!buf)
        {
            LOG_ERROR("Incorrect buffer provided at slot %d", i);
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
    Buffer* ib = dynamic_cast<Buffer*>(indexBuffer.get());
    if (ib == nullptr)
    {
        LOG_ERROR("Incorrect Index Buffer provided");
        return;
    }

    vkCmdBindIndexBuffer(mCommandBuffer, ib->mBuffer, 0, TranslateIndexBufferFormatToVkIndexType(format));
}

void CommandRecorder::BindResources(size_t slot, const ResourceBindingInstancePtr& bindingSetInstance)
{
    UNUSED(slot);

    ResourceBindingInstance* rbi = dynamic_cast<ResourceBindingInstance*>(bindingSetInstance.get());
    if (rbi == nullptr)
    {
        LOG_ERROR("Incorrect resource binding instance provided");
        return;
    }

    vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            mResourceBindingLayout->mPipelineLayout, rbi->mSet->mSetSlot, 1,
                            &rbi->mSet->mDescriptorSet, 0, nullptr);
}

void CommandRecorder::BindVolatileCBuffer(size_t slot, const BufferPtr& buffer)
{
    Buffer* b = dynamic_cast<Buffer*>(buffer.get());
    if (b == nullptr)
    {
        LOG_ERROR("Invalid volatile buffer provided");
        return;
    }

    if (b->mMode != BufferMode::Volatile)
    {
        LOG_ERROR("Buffer with invalid mode provided");
        return;
    }

    if (slot >= VK_MAX_VOLATILE_BUFFERS)
    {
        LOG_ERROR("Binding to slot %d impossible (max available slots 0-7).", slot);
        return;
    }

    if (b != mBoundVolatileBuffers[slot])
    {
        mBoundVolatileBuffers[slot] = b;
    }
}

void CommandRecorder::SetResourceBindingLayout(const ResourceBindingLayoutPtr& layout)
{
    mResourceBindingLayout = dynamic_cast<ResourceBindingLayout*>(layout.get());
    if (!mResourceBindingLayout)
        LOG_ERROR("Incorrect binding layout provided");
}

void CommandRecorder::SetRenderTarget(const RenderTargetPtr& renderTarget)
{
    if (mRenderTarget)
    {
        // there is a previous render pass active, end it
        vkCmdEndRenderPass(mCommandBuffer);
    }

    mRenderTarget = dynamic_cast<RenderTarget*>(renderTarget.get());
    if (!mRenderTarget)
    {
        LOG_ERROR("Incorrect Render Target pointer.");
        return;
    }

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
    PipelineState* ps = dynamic_cast<PipelineState*>(state.get());
    if (ps == nullptr)
    {
        LOG_ERROR("Incorrect pipeline state provided");
        return;
    }

    // TODO support compute bind point
    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ps->mPipeline);
}

void CommandRecorder::SetStencilRef(unsigned char ref)
{
    UNUSED(ref);
}

void CommandRecorder::SetViewport(float left, float width, float top, float height,
                                float minDepth, float maxDepth)
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

void CommandRecorder::SetScissors(int left, int top, int right, int bottom)
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
    UNUSED(buffer);
    UNUSED(type);
    return nullptr;
}

void CommandRecorder::UnmapBuffer(const BufferPtr& buffer)
{
    UNUSED(buffer);
}

bool CommandRecorder::WriteDynamicBuffer(Buffer* b, size_t offset, size_t size, const void* data)
{
    // copy data to Ring Buffer
    uint32 sourceOffset = gDevice->GetRingBuffer()->Write(data, static_cast<uint32>(size));

    if (sourceOffset == std::numeric_limits<uint32>::max())
    {
        LOG_ERROR("Failed to write temporary data to Ring Ruffer - the Ring Buffer is full");
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
    uint32 writeHead = gDevice->GetRingBuffer()->Write(data, static_cast<uint32>(size));
    if (writeHead == std::numeric_limits<uint32>::max())
    {
        LOG_ERROR("Failed to write data to Ring Ruffer - the Ring Buffer is full");
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
    Buffer* b = dynamic_cast<Buffer*>(buffer.get());
    if (b == nullptr)
    {
        LOG_ERROR("Invalid buffer pointer provided");
        return false;
    }

    if (b->mMode == BufferMode::Dynamic)
    {
        if (static_cast<VkDeviceSize>(size) > b->mBufferSize)
        {
            LOG_ERROR("Requested to write more than provided buffer can handle (%d vs buffer's $d)",
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
        LOG_ERROR("Provided buffer does not have a CPU-writable mode");
        return false;
    }
}

void CommandRecorder::CopyTexture(const TexturePtr& src, const TexturePtr& dest)
{
    UNUSED(src);
    UNUSED(dest);
}

void CommandRecorder::Clear(int flags, uint32 numTargets, const uint32* slots,
                            const Math::Float4* colors, float depthValue, uint8 stencilValue)
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


void CommandRecorder::Draw(int vertexNum, int instancesNum, int vertexOffset,
                         int instanceOffset)
{
    vkCmdDraw(mCommandBuffer, vertexNum, instancesNum, vertexOffset, instanceOffset);
}

void CommandRecorder::DrawIndexed(int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    vkCmdDrawIndexed(mCommandBuffer, indexNum, instancesNum, indexOffset, vertexOffset, instanceOffset);
}

void CommandRecorder::BindComputeResources(size_t slot, const ResourceBindingInstancePtr& bindingSetInstance)
{
    UNUSED(slot);
    UNUSED(bindingSetInstance);
}

void CommandRecorder::BindComputeVolatileCBuffer(size_t slot, const BufferPtr& buffer)
{
    UNUSED(slot);
    UNUSED(buffer);
}

void CommandRecorder::SetComputeResourceBindingLayout(const ResourceBindingLayoutPtr& layout)
{
    UNUSED(layout);
}

void CommandRecorder::SetComputePipelineState(const ComputePipelineStatePtr& state)
{
    UNUSED(state);
}

void CommandRecorder::Dispatch(uint32 x, uint32 y, uint32 z)
{
    UNUSED(x);
    UNUSED(y);
    UNUSED(z);
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
        LOG_ERROR("Error during Constant Buffer recording: %d (%s)",
                  result, TranslateVkResultToString(result));
        return 0;
    }

    return gDevice->GetCurrentCommandBuffer() + 1;
}

void CommandRecorder::BeginDebugGroup(const char* text)
{
    UNUSED(text);
}

void CommandRecorder::EndDebugGroup()
{
}

void CommandRecorder::InsertDebugMarker(const char* text)
{
    UNUSED(text);
}

} // namespace Renderer
} // namespace NFE

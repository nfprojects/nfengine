/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Command Buffer
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Device.hpp"
#include "Translations.hpp"
#include "CommandBuffer.hpp"
#include "RenderTarget.hpp"
#include "Buffer.hpp"

#include <string.h>


namespace NFE {
namespace Renderer {


void CommandBuffer::UpdateStates()
{
    const FullPipelineStateParts parts(mPipelineState, mShaderProgram);

}

CommandBuffer::CommandBuffer()
    : mCommandBuffer(VK_NULL_HANDLE)
    , mRenderTarget(nullptr)
{
}

CommandBuffer::~CommandBuffer()
{
    if (mCommandBuffer)
        vkFreeCommandBuffers(gDevice->GetDevice(), gDevice->GetCommandPool(),
                             1, &mCommandBuffer);
}

bool CommandBuffer::Init()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    VK_ZERO_MEMORY(allocInfo);
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = gDevice->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    VkResult result = vkAllocateCommandBuffers(gDevice->GetDevice(), &allocInfo, &mCommandBuffer);
    CHECK_VKRESULT(result, "Failed to allocate a command buffer");

    VK_ZERO_MEMORY(mCommandBufferBeginInfo);
    mCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    mCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    LOG_INFO("Command Buffer initialized successfully.");
    return true;
}

void CommandBuffer::Reset()
{
    gDevice->WaitForGPU();
    mRenderTarget = nullptr;
    vkResetCommandBuffer(mCommandBuffer, 0);
    vkBeginCommandBuffer(mCommandBuffer, &mCommandBufferBeginInfo);
}

void CommandBuffer::SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets)
{
    UNUSED(strides);

    const int maxBuffers = 4;
    VkBuffer buffers[maxBuffers];
    VkDeviceSize offs[maxBuffers];

    for (int i = 0; i < num; ++i)
    {
        Buffer* buf = dynamic_cast<Buffer*>(vertexBuffers[i]);
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

void CommandBuffer::SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format)
{
    UNUSED(indexBuffer);
    UNUSED(format);
}

void CommandBuffer::BindResources(size_t slot, IResourceBindingInstance* bindingSetInstance)
{
    UNUSED(slot);
    UNUSED(bindingSetInstance);
}

void CommandBuffer::BindDynamicBuffer(size_t slot, IBuffer* buffer)
{
    UNUSED(slot);
    UNUSED(buffer);
}

void CommandBuffer::SetResourceBindingLayout(IResourceBindingLayout* layout)
{
    UNUSED(layout);
}

void CommandBuffer::SetRenderTarget(IRenderTarget* renderTarget)
{
    if (mRenderTarget)
    {
        // there is a previous render pass active, end it
        vkCmdEndRenderPass(mCommandBuffer);
    }

    mRenderTarget = dynamic_cast<RenderTarget*>(renderTarget);
    if (!mRenderTarget)
    {
        LOG_ERROR("Incorrect Render Target pointer.");
        return;
    }

    if (!mRenderTarget) {
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

void CommandBuffer::SetShaderProgram(IShaderProgram* shaderProgram)
{
    ShaderProgram* sp = dynamic_cast<ShaderProgram*>(shaderProgram);
    if (sp == nullptr)
    {
        LOG_ERROR("Incorrect pipeline state provided");
        return;
    }

    mShaderProgram = sp;
    mUpdatePipeline = true;
}

void CommandBuffer::SetPipelineState(IPipelineState* state)
{
    PipelineState* ps = dynamic_cast<PipelineState*>(state);
    if (ps == nullptr)
    {
        LOG_ERROR("Incorrect pipeline state provided");
        return;
    }

    mPipelineState = ps;
    mUpdatePipeline = true;
}

void CommandBuffer::SetStencilRef(unsigned char ref)
{
    UNUSED(ref);
}

void CommandBuffer::SetViewport(float left, float width, float top, float height,
                                float minDepth, float maxDepth)
{
    VkViewport viewport;
    memset(&viewport, 0, sizeof(viewport));
    viewport.x = left;
    viewport.y = top;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
}

void CommandBuffer::SetScissors(int left, int top, int right, int bottom)
{
    VkRect2D scissor;
    scissor.offset.x = left;
    scissor.offset.y = top;
    scissor.extent.width = right - left;
    scissor.extent.height = bottom - top;
    vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
}

void* CommandBuffer::MapBuffer(IBuffer* buffer, MapType type)
{
    UNUSED(buffer);
    UNUSED(type);
    return nullptr;
}

void CommandBuffer::UnmapBuffer(IBuffer* buffer)
{
    UNUSED(buffer);
}

bool CommandBuffer::WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data)
{
    UNUSED(buffer);
    UNUSED(offset);
    UNUSED(size);
    UNUSED(data);

    return false;
}

void CommandBuffer::CopyTexture(ITexture* src, ITexture* dest)
{
    UNUSED(src);
    UNUSED(dest);
}

void CommandBuffer::Clear(int flags, const float* color, float depthValue,
                          unsigned char stencilValue)
{
    if (!mRenderTarget)
        return;

    std::vector<VkClearAttachment> clearAtts;

    if (flags & NFE_CLEAR_FLAG_TARGET)
    {
        clearAtts.resize(mRenderTarget->mAttachmentCount);
        for (uint32 i = 0; i < mRenderTarget->mAttachmentCount; ++i)
        {
            clearAtts[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            clearAtts[i].colorAttachment = i;
            clearAtts[i].clearValue.color.float32[0] = color[0];
            clearAtts[i].clearValue.color.float32[1] = color[1];
            clearAtts[i].clearValue.color.float32[2] = color[2];
            clearAtts[i].clearValue.color.float32[3] = color[3];
        }
    }

    if (flags & NFE_CLEAR_FLAG_DEPTH)
    {
        VkClearAttachment depth;
        depth.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depth.clearValue.depthStencil.depth = depthValue;
        clearAtts.push_back(depth);
    }

    if (flags & NFE_CLEAR_FLAG_STENCIL)
    {
        VkClearAttachment stencil;
        stencil.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        stencil.clearValue.depthStencil.stencil = stencilValue;
        clearAtts.push_back(stencil);
    }

    VkClearRect rect;
    rect.rect.offset = { 0, 0 };
    rect.rect.extent = { static_cast<uint32>(mRenderTarget->mWidth),
                         static_cast<uint32>(mRenderTarget->mHeight) };
    rect.baseArrayLayer = 0;
    rect.layerCount = 1;
    vkCmdClearAttachments(mCommandBuffer, static_cast<uint32>(clearAtts.size()), clearAtts.data(),
                          1, &rect);
}


void CommandBuffer::Draw(int vertexNum, int instancesNum, int vertexOffset,
                         int instanceOffset)
{
    UNUSED(vertexNum);
    UNUSED(instancesNum);
    UNUSED(vertexOffset);
    UNUSED(instanceOffset);
}

void CommandBuffer::DrawIndexed(int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    UNUSED(indexNum);
    UNUSED(instancesNum);
    UNUSED(indexOffset);
    UNUSED(vertexOffset);
    UNUSED(instanceOffset);
}

std::unique_ptr<ICommandList> CommandBuffer::Finish()
{
    if (mRenderTarget)
    {
        vkCmdEndRenderPass(mCommandBuffer);

        // barrier for our backbuffer, pre-present
        VkImageMemoryBarrier prePresentBarrier;
        VK_ZERO_MEMORY(prePresentBarrier);
        prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        prePresentBarrier.subresourceRange.baseMipLevel = 0;
        prePresentBarrier.subresourceRange.levelCount = 1;
        prePresentBarrier.subresourceRange.baseArrayLayer = 0;
        prePresentBarrier.subresourceRange.layerCount = 1;
        prePresentBarrier.image = mRenderTarget->mTex->mBuffers[mRenderTarget->mTex->mCurrentBuffer];
        vkCmdPipelineBarrier(mCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &prePresentBarrier);
    }

    VkResult result = vkEndCommandBuffer(mCommandBuffer);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Error during Constant Buffer recording: %d (%s)",
                  result, TranslateVkResultToString(result));
        return nullptr;
    }

    std::unique_ptr<CommandList> list(new (std::nothrow) CommandList);
    if (!list)
    {
        LOG_ERROR("Failed to allocate memory for command list");
        return nullptr;
    }

    list->cmdBuffer = this;
    return list;
}

void CommandBuffer::BeginDebugGroup(const char* text)
{
    UNUSED(text);
}

void CommandBuffer::EndDebugGroup()
{
}

void CommandBuffer::InsertDebugMarker(const char* text)
{
    UNUSED(text);
}

} // namespace Renderer
} // namespace NFE

#pragma once

#include "ICommand.hpp"

#include "API/Device.hpp"

#include "../RendererCommon/Types.hpp"


namespace NFE {
namespace Renderer {


class EndRenderPassCommand: public ICommand
{
public:
    EndRenderPassCommand()
        : ICommand(50)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdEndRenderPass(commandBuffer);
    }

    const char* GetName() const override
    {
        return "EndRenderPass";
    }
};



class BindPipelineLayoutCommand: public ICommand
{
    VkPipelineLayout mPipelineLayout;

public:
    BindPipelineLayoutCommand(VkPipelineLayout pipelineLayout)
        : ICommand(100)
        , mPipelineLayout(pipelineLayout)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(commandBuffer);
        state.mPipelineLayout = mPipelineLayout;
    }

    const char* GetName() const override
    {
        return "BindPipelineLayout";
    }
};

class BindPipelineCommand: public ICommand
{
    VkPipelineBindPoint mBindPoint;
    VkPipeline mPipeline;

public:
    BindPipelineCommand(VkPipelineBindPoint bindPoint, VkPipeline pipeline)
        : ICommand(100)
        , mBindPoint(bindPoint)
        , mPipeline(pipeline)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdBindPipeline(commandBuffer, mBindPoint, mPipeline);
    }

    const char* GetName() const override
    {
        return "BindPipeline";
    }
};



class BindDescriptorSetCommand: public ICommand
{
    VkPipelineBindPoint mBindPoint;
    uint32 mSetSlot;
    VkDescriptorSet mSet;

public:
    BindDescriptorSetCommand(VkPipelineBindPoint bindPoint, uint32 setSlot, VkDescriptorSet set)
        : ICommand(200)
        , mBindPoint(bindPoint)
        , mSetSlot(setSlot)
        , mSet(set)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        vkCmdBindDescriptorSets(commandBuffer, mBindPoint,
                                state.mPipelineLayout,
                                mSetSlot, 1, &mSet, 0, nullptr);
    }

    const char* GetName() const override
    {
        return "BindDescriptorSet";
    }
};

class BindVolatileDescriptorSetCommand: public ICommand
{
    VkPipelineBindPoint mBindPoint;
    uint32 mSetSlot;
    uint32 mOffsetCount;
    uint32 mOffsets[NFE_VK_MAX_VOLATILE_BUFFERS];

public:
    BindVolatileDescriptorSetCommand(VkPipelineBindPoint bindPoint, uint32 setSlot, uint32 offsetCount, uint32* offsets)
        : ICommand(200)
        , mBindPoint(bindPoint)
        , mSetSlot(setSlot)
        , mOffsetCount(offsetCount)
    {
        NFE_ASSERT(mOffsetCount <= NFE_VK_MAX_VOLATILE_BUFFERS, "Too many Volatile Buffers to bind");
        memcpy(mOffsets, offsets, sizeof(uint32) * mOffsetCount);
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        // TODO
        vkCmdBindDescriptorSets(commandBuffer, mBindPoint,
                                state.mPipelineLayout,
                                mSetSlot, 0, nullptr, 0, nullptr);
    }

    const char* GetName() const override
    {
        return "BindVolatileDescriptorSet";
    }
};

class BindVertexBuffersCommand: public ICommand
{
    uint32 mFirstBinding;
    uint32 mBindingCount;
    VkBuffer mBuffers[NFE_VK_MAX_VERTEX_BUFFERS];
    VkDeviceSize mOffsets[NFE_VK_MAX_VERTEX_BUFFERS];

public:
    BindVertexBuffersCommand(uint32 firstBinding, uint32 bindingCount, VkBuffer* buffers, VkDeviceSize* offsets)
        : ICommand(200)
        , mFirstBinding(firstBinding)
        , mBindingCount(bindingCount)
    {
        NFE_ASSERT(mBindingCount <= NFE_VK_MAX_VERTEX_BUFFERS, "Too many Vertex Buffers to bind");
        memcpy(mBuffers, buffers, sizeof(VkBuffer) * bindingCount);
        memcpy(mOffsets, offsets, sizeof(VkDeviceSize) * bindingCount);
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdBindVertexBuffers(commandBuffer, mFirstBinding, mBindingCount, mBuffers, mOffsets);
    }

    const char* GetName() const override
    {
        return "BindVertexBuffers";
    }
};

class BindIndexBufferCommand: public ICommand
{
    VkBuffer mBuffer;
    VkDeviceSize mOffset;
    VkIndexType mIndexType;

public:
    BindIndexBufferCommand(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
        : ICommand(200)
        , mBuffer(buffer)
        , mOffset(offset)
        , mIndexType(indexType)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdBindIndexBuffer(commandBuffer, mBuffer, mOffset, mIndexType);
    }

    const char* GetName() const override
    {
        return "BindIndexBuffer";
    }
};

// TODO COMPLETE
class PipelineBarrierCommand: public ICommand
{
    VkPipelineStageFlags mSourceStage;
    VkPipelineStageFlags mDestinationStage;
    uint32 mImageBarrierCount;
    VkImageMemoryBarrier mImageBarriers[NFE_VK_MAX_BARRIERS];

public:
    PipelineBarrierCommand(VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage,
                           uint32 imageBarrierCount, VkImageMemoryBarrier* imageBarriers)
        : ICommand(300)
        , mSourceStage(sourceStage)
        , mDestinationStage(destinationStage)
        , mImageBarrierCount(imageBarrierCount)
    {
        NFE_ASSERT(imageBarrierCount <= NFE_VK_MAX_BARRIERS, "Too many image barriers provided");
        memcpy(mImageBarriers, imageBarriers, sizeof(VkImageMemoryBarrier) * mImageBarrierCount);
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdPipelineBarrier(commandBuffer,
                             mSourceStage, mDestinationStage, 0,
                             0, nullptr, 0, nullptr,
                             mImageBarrierCount, mImageBarriers);
    }

    const char* GetName() const override
    {
        return "PipelineBarrier";
    }
};



class BeginRenderPassCommand: public ICommand
{
    VkRenderPassBeginInfo mInfo;

public:
    BeginRenderPassCommand(const VkRenderPassBeginInfo& info)
        : ICommand(400)
        , mInfo(info)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        state.mRenderTargetRect = mInfo.renderArea;
        vkCmdBeginRenderPass(commandBuffer, &mInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    const char* GetName() const override
    {
        return "BeginRenderPass";
    }
};



class ClearCommand: public ICommand
{
    VkClearAttachment mClearAttachments[MAX_RENDER_TARGETS + 2];
    uint32 mClearAttachmentsNum;

public:
    ClearCommand(VkClearAttachment* clearAttachments, uint32 clearAttachmentsNum)
        : ICommand(500)
        , mClearAttachmentsNum(clearAttachmentsNum)
    {
        NFE_ASSERT(mClearAttachmentsNum <= (MAX_RENDER_TARGETS + 2), "Too many attachments to clear (%d vs max %d)",
                                                                     mClearAttachmentsNum, MAX_RENDER_TARGETS + 2);
        memcpy(mClearAttachments, clearAttachments, sizeof(VkClearAttachment) * mClearAttachmentsNum);
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        VkClearRect clearRect;
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;
        clearRect.rect = state.mRenderTargetRect;
        vkCmdClearAttachments(commandBuffer, mClearAttachmentsNum, mClearAttachments, 1, &clearRect);
    }

    const char* GetName() const override
    {
        return "Clear";
    }
};

class SetScissorCommand: public ICommand
{
    VkRect2D mScissor;

public:
    SetScissorCommand(const VkRect2D& scissor)
        : ICommand(500)
        , mScissor(scissor)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdSetScissor(commandBuffer, 0, 1, &mScissor);
    }

    const char* GetName() const override
    {
        return "SetScissor";
    }
};

class SetStencilRefCommand: public ICommand
{
    uint32 mReference;

public:
    SetStencilRefCommand(uint32 reference)
        : ICommand(500)
        , mReference(reference)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdSetStencilReference(commandBuffer, VK_STENCIL_FACE_FRONT_BIT | VK_STENCIL_FACE_BACK_BIT, mReference);
    }

    const char* GetName() const override
    {
        return "SetStencilRef";
    }
};

class SetViewportCommand: public ICommand
{
    VkViewport mViewport;

public:
    SetViewportCommand(const VkViewport& viewport)
        : ICommand(500)
        , mViewport(viewport)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdSetViewport(commandBuffer, 0, 1, &mViewport);
    }

    const char* GetName() const override
    {
        return "SetViewport";
    }
};

class BlitTextureCommand: public ICommand
{
    VkImage mSrcImage, mDstImage;
    VkImageBlit mRegion;

public:
    BlitTextureCommand(VkImage srcImage, VkImage dstImage, const VkImageBlit& region)
        : ICommand(900)
        , mSrcImage(srcImage)
        , mDstImage(dstImage)
        , mRegion(region)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdBlitImage(commandBuffer,
                       mSrcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       mDstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &mRegion, VK_FILTER_NEAREST);
    }

    const char* GetName() const override
    {
        return "BlitTexture";
    }
};

class CopyTextureCommand: public ICommand
{
    VkImage mSrcImage, mDstImage;
    VkImageCopy mRegion;

public:
    CopyTextureCommand(VkImage srcImage, VkImage dstImage, const VkImageCopy& region)
        : ICommand(900)
        , mSrcImage(srcImage)
        , mDstImage(dstImage)
        , mRegion(region)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdCopyImage(commandBuffer,
                       mSrcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       mDstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &mRegion);
    }

    const char* GetName() const override
    {
        return "CopyTexture";
    }
};

class DispatchCommand: public ICommand
{
    uint32 mX, mY, mZ;

public:
    DispatchCommand(uint32 x, uint32 y, uint32 z)
        : ICommand(900)
        , mX(x)
        , mY(y)
        , mZ(z)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdDispatch(commandBuffer, mX, mY, mZ);
    }

    const char* GetName() const override
    {
        return "Dispatch";
    }
};

class DrawCommand: public ICommand
{
    uint32 mVertexNum;
    uint32 mInstancesNum;
    uint32 mVertexOffset;
    uint32 mInstanceOffset;

public:
    DrawCommand(uint32 vertexNum, uint32 instancesNum, uint32 vertexOffset, uint32 instanceOffset)
        : ICommand(900)
        , mVertexNum(vertexNum)
        , mInstancesNum(instancesNum)
        , mVertexOffset(vertexOffset)
        , mInstanceOffset(instanceOffset)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdDraw(commandBuffer, mVertexNum, mInstancesNum, mVertexOffset, mInstanceOffset);
    }

    const char* GetName() const override
    {
        return "Draw";
    }
};

class DrawIndexedCommand: public ICommand
{
    uint32 mIndexNum;
    uint32 mInstancesNum;
    uint32 mIndexOffset;
    uint32 mVertexOffset;
    uint32 mInstanceOffset;

public:
    DrawIndexedCommand(uint32 indexNum, uint32 instancesNum, uint32 indexOffset, int32 vertexOffset, uint32 instanceOffset)
        : ICommand(900)
        , mIndexNum(indexNum)
        , mInstancesNum(instancesNum)
        , mIndexOffset(indexOffset)
        , mVertexOffset(vertexOffset)
        , mInstanceOffset(instanceOffset)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(state);
        vkCmdDrawIndexed(commandBuffer, mIndexNum, mInstancesNum, mIndexOffset, mVertexOffset, mInstanceOffset);
    }

    const char* GetName() const override
    {
        return "DrawIndexed";
    }
};

class WriteBufferCommand: public ICommand
{
public:
    WriteBufferCommand()
        : ICommand(900)
    {
    }

    void Execute(VkCommandBuffer commandBuffer, CommandBufferState& state) override
    {
        NFE_UNUSED(commandBuffer);
        NFE_UNUSED(state);
    }

    const char* GetName() const override
    {
        return "WriteBuffer";
    }
};


} // namespace Renderer
} // namespace NFE

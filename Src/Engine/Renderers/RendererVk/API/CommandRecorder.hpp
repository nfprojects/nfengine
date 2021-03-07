/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Command Recorder
 */

#pragma once

#include "../RendererCommon/CommandRecorder.hpp"

#include "Defines.hpp"
#include "RenderTarget.hpp"
#include "PipelineState.hpp"
#include "Buffer.hpp"
#include "Internal/RingBuffer.hpp"


namespace NFE {
namespace Renderer {

class CommandRecorder: public ICommandRecorder
{
    friend class Device;

    // General fields
    VkCommandBuffer mCommandBuffer;
    VkCommandBufferBeginInfo mCommandBufferBeginInfo;

    // Graphics resources
    RenderTarget* mRenderTarget;
    bool mActiveRenderPass;
    ResourceBindingLayout* mResourceBindingLayout;
    Buffer* mBoundVolatileBuffers[VK_MAX_VOLATILE_BUFFERS];
    uint32 mBoundVolatileOffsets[VK_MAX_VOLATILE_BUFFERS];
    bool mRebindDynamicBuffers;

    // Compute resources
    ResourceBindingLayout* mComputeResourceBindingLayout;

    bool WriteDynamicBuffer(Buffer* b, size_t offset, size_t size, const void* data);
    bool WriteVolatileBuffer(Buffer* b, size_t size, const void* data);
    void RebindDynamicBuffers() const;


public:
    CommandRecorder();
    ~CommandRecorder();

    bool Init();

    /// Common methods
    bool Begin(CommandQueueType queueType) override;
    void CopyBuffer(const BufferPtr& src, const BufferPtr& dest, size_t size, size_t srcOffset, size_t destOffset) override;
    void CopyTexture(const TexturePtr& src, const TexturePtr& dest) override;
    void CopyTexture(const TexturePtr& src, const BackbufferPtr& dest) override;
    void CopyTextureToBuffer(const TexturePtr& src, const BufferPtr& dest, const TextureRegion* texRegion, uint32 bufferOffset) override;
    bool WriteBuffer(const BufferPtr& buffer, size_t offset, size_t size, const void* data) override;
    bool WriteTexture(const TexturePtr& texture, const void* data, const TextureRegion* texRegion, uint32 srcRowStride) override;
    CommandListPtr Finish() override;

    /// Commands/calls
    void BindResources(PipelineType pipelineType, uint32 setIndex, const ResourceBindingInstancePtr& bindingSetInstance) override;
    void BindBuffer(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const BufferPtr& buffer, const BufferView& view) override;
    void BindTexture(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const TexturePtr& texture, const TextureView& view) override;
    void BindVolatileCBuffer(PipelineType pipelineType, uint32 slot, const BufferPtr& buffer) override;
    void BindWritableBuffer(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const BufferPtr& buffer, const BufferView& view) override;
    void BindWritableTexture(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const TexturePtr& texture, const TextureView& view) override;
    void Clear(uint32 flags, uint32 numTargets, const uint32* slots, const Math::Vec4fU* colors, float depthValue, uint8 stencilValue) override;
    void SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format) override;
    void SetRenderTarget(const RenderTargetPtr& renderTarget) override;
    void SetPipelineState(const PipelineStatePtr& state) override;
    void SetComputePipelineState(const ComputePipelineStatePtr& state) override;
    void SetResourceBindingLayout(PipelineType pipelineType, const ResourceBindingLayoutPtr& layout) override;
    void SetScissors(int32 left, int32 top, int32 right, int32 bottom) override;
    void SetStencilRef(unsigned char ref) override;
    void SetVertexBuffers(uint32 num, const BufferPtr* vertexBuffers, uint32* strides, uint32* offsets) override;
    void SetViewport(float left, float width, float top, float height, float minDepth, float maxDepth) override;
    void Draw(uint32 vertexNum, uint32 instancesNum, uint32 vertexOffset, uint32 instanceOffset) override;
    void DrawIndexed(uint32 indexNum, uint32 instancesNum, uint32 indexOffset, int32 vertexOffset, uint32 instanceOffset) override;
    void Dispatch(uint32 x, uint32 y, uint32 z) override;
    void DispatchIndirect(const BufferPtr& indirectArgBuffer, uint32 bufferOffset) override;

    // Hints for resource transition between queues
    void HintTargetCommandQueueType(const BufferPtr& resource, const CommandQueueType targetType) override;
    void HintTargetCommandQueueType(const TexturePtr& resource, const CommandQueueType targetType) override;

    /// Debugging
    void BeginDebugGroup(const char* text) override;
    void EndDebugGroup() override;
    void InsertDebugMarker(const char* text) override;
};

} // namespace Renderer
} // namespace NFE

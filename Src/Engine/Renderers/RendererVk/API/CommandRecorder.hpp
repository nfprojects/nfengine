/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Command Recorder
 */

#pragma once

#include "../RendererCommon/CommandRecorder.hpp"
#include "../RendererCommon/Types.hpp"
#include "../RendererCommon/Shader.hpp"

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

    // order here should match PipelineState::RemapDescriptorSets
    enum class ShaderResourceType: uint8
    {
        UniformBuffer = 0,
        SampledImage,
        Sampler,
        StorageBuffer,
        StorageImage,
    };

    struct TemporaryResourceBind
    {
        IResource* resource;
        ShaderType stage;
        ShaderResourceType type;
        uint32 slot;

        TemporaryResourceBind(IResource* r, ShaderType stage, ShaderResourceType type, uint32 set)
            : resource(r)
            , stage(stage)
            , type(type)
            , slot(set)
        {
        }
    };

    using PendingResourcesArray = Common::StaticArray<TemporaryResourceBind, VK_MAX_PENDING_RESOURCES>;
    using DescriptorSetArray = Common::StaticArray<VkDescriptorSet, VK_MAX_DESCRIPTOR_SETS>;

    // General fields
    VkCommandBuffer mCommandBuffer;
    CommandQueueType mQueueType;
    VkCommandBufferBeginInfo mCommandBufferBeginInfo;

    PipelineState* mPipelineState;
    PendingResourcesArray mPendingResources;
    DescriptorSetArray mTemporaryDescriptorSets;

    // Graphics resources
    RenderTarget* mRenderTarget;
    bool mActiveRenderPass;
    Buffer* mBoundVolatileBuffers[VK_MAX_VOLATILE_BUFFERS];
    uint32 mBoundVolatileOffsets[VK_MAX_VOLATILE_BUFFERS];
    bool mRebindDynamicBuffers;

    void EnsureOutsideRenderPass();
    void EnsureInsideRenderPass();
    bool WriteDynamicBuffer(Buffer* b, size_t offset, size_t size, const void* data);
    bool WriteVolatileBuffer(Buffer* b, size_t size, const void* data);
    void RebindDynamicBuffers() const;

    NFE_INLINE void StoreDescriptorSetBinding(uint32 slot, VkDescriptorSet set)
    {
        mTemporaryDescriptorSets[slot] = set;
    }

    uint32 AcquireTargetDescriptorSetIdx(ShaderType stage, ShaderResourceType type);
    void ClearDescriptorSetBindings();
    void BindPendingResources();

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
    void BindBuffer(ShaderType stage, uint32 slot, const BufferPtr& buffer, const BufferView& view) override;
    void BindConstantBuffer(ShaderType stage, uint32 slot, const BufferPtr& buffer) override;
    void BindTexture(ShaderType stage, uint32 slot, const TexturePtr& texture, const TextureView& view) override;
    void BindSampler(ShaderType stage, uint32 slot, const SamplerPtr& sampler) override;
    void BindWritableBuffer(ShaderType stage, uint32 slot, const BufferPtr& buffer, const BufferView& view) override;
    void BindWritableTexture(ShaderType stage, uint32 slot, const TexturePtr& texture, const TextureView& view) override;
    void Clear(uint32 flags, uint32 numTargets, const uint32* slots, const Math::Vec4fU* colors, float depthValue, uint8 stencilValue) override;
    void SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format) override;
    void SetRenderTarget(const RenderTargetPtr& renderTarget) override;
    void SetPipelineState(const PipelineStatePtr& state) override;
    void SetComputePipelineState(const ComputePipelineStatePtr& state) override;
    void SetScissors(int32 left, int32 top, int32 right, int32 bottom) override;
    void SetStencilRef(unsigned char ref) override;
    void SetVertexBuffers(uint32 num, const BufferPtr* vertexBuffers, const uint32* strides, const uint32* offsets) override;
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

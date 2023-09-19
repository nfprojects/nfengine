/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's command recorder.
 */

#pragma once

#include "../RendererCommon/CommandRecorder.hpp"
#include "Common.hpp"
#include "RingBuffer.hpp"
#include "CommandList.hpp"
#include "ResourceStateCache.hpp"
#include "HeapAllocator.hpp"
#include "DescriptorSet.hpp"

#include "Engine/Common/Containers/HashSet.hpp"
#include "Engine/Common/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

class RenderTarget;
class PipelineState;
class ComputePipelineState;
class Buffer;
class InternalCommandList;
enum class CommandQueueType : uint8;

class CommandRecorder : public ICommandRecorder
{
public:
    CommandRecorder();
    ~CommandRecorder();

    bool Begin(CommandQueueType queueType) override;
    CommandListPtr Finish() override;

    bool WriteBuffer(const BufferPtr& buffer, size_t offset, size_t size, const void* data) override;
    bool WriteTexture(const TexturePtr& texture, const void* data, const TextureRegion* texRegion, uint32 srcRowStride) override;
    void CopyBuffer(const BufferPtr& src, const BufferPtr& dest, size_t size, size_t srcOffset, size_t destOffset) override;
    void CopyTexture(const TexturePtr& src, const TexturePtr& dest) override;
    void CopyTextureToBuffer(const TexturePtr& src, const BufferPtr& dest, const TextureRegion* texRegion, uint32 bufferOffset);
    void CopyTexture(const TexturePtr& src, const BackbufferPtr& dest) override;

    /// Resource binding methods
    void BindTexture(ShaderType stage, uint32 slot, const TexturePtr& texture, const TextureView& view) override;
    void BindWritableTexture(ShaderType stage, uint32 slot, const TexturePtr& texture, const TextureView& view) override;
    void BindBuffer(ShaderType stage, uint32 slot, const BufferPtr& buffer, const BufferView& view) override;
    void BindWritableBuffer(ShaderType stage, uint32 slot, const BufferPtr& buffer, const BufferView& view) override;
    void BindConstantBuffer(ShaderType stage, uint32 slot, const BufferPtr& buffer) override;
    void BindSampler(ShaderType stage, uint32 slot, const SamplerPtr& sampler) override;

    /// Graphics pipeline methods
    void SetVertexBuffers(uint32 num, const BufferPtr* vertexBuffers, const uint32* strides, const uint32* offsets) override;
    void SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format) override;
    void SetRenderTarget(const RenderTargetPtr& renderTarget) override;
    void SetPipelineState(const PipelineStatePtr& state) override;
    void SetStencilRef(uint8 ref) override;
    void SetViewport(float left, float width, float top, float height, float minDepth, float maxDepth) override;
    void SetScissors(int32 left, int32 top, int32 right, int32 bottom) override;
    void Clear(uint32 flags, uint32 numTargets, const uint32* slots, const Math::Vec4fU* colors, float depthValue, uint8 stencilValue) override;
    void Draw(uint32 vertexNum, uint32 instancesNum, uint32 vertexOffset, uint32 instanceOffset) override;
    void DrawIndexed(uint32 indexNum, uint32 instancesNum, uint32 indexOffset, int32 vertexOffset, uint32 instanceOffset) override;

    /// Compute pipeline methods
    void SetComputePipelineState(const ComputePipelineStatePtr& state) override;
    void Dispatch(uint32 x, uint32 y, uint32 z) override;
    void DispatchIndirect(const BufferPtr& indirectArgBuffer, uint32 bufferOffset) override;

    /// Misc
    void HintTargetCommandQueueType(const BufferPtr& resource, const CommandQueueType targetType) override;
    void HintTargetCommandQueueType(const TexturePtr& resource, const CommandQueueType targetType) override;

    /// Debugging
    void BeginDebugGroup(const char* text) override;
    void EndDebugGroup() override;
    void InsertDebugMarker(const char* text) override;

private:

    void ResetState();

    void Internal_UpdateGraphicsPipelineState();
    void Internal_UpdateVetexAndIndexBuffers();
    void Internal_PrepareForDraw();

    void Internal_UpdateComputePipelineState();
    void Internal_PrepareForDispatch();

    void Internal_UnsetRenderTarget();

    void Internal_WriteBuffer(Buffer* buffer, size_t offset, size_t size, const void* data);
    void Internal_WriteVolatileBuffer(Buffer* buffer, const void* data);

    ReferencedResourcesList& Internal_GetReferencedResources();

    CommandQueueType mQueueType;
    InternalCommandListPtr mCommandListObject;
    ID3D12GraphicsCommandList* mCommandList; // cached D3D12 command list pointer

    ResourceStateCache mResourceStateCache;

    const RenderTarget* mCurrRenderTarget;
    const PipelineState* mGraphicsPipelineState;
    const ComputePipelineState* mComputePipelineState;

    D3D12_PRIMITIVE_TOPOLOGY mCurrPrimitiveTopology;

    bool mGraphicsPipelineStateChanged : 1;
    bool mComputePipelineStateChanged : 1;
    bool mVertexBufferChanged : 1;
    bool mIndexBufferChanged : 1;

    DescriptorSet mDescriptorSets[NFE_RENDERER_MAX_SHADER_TYPES];

    D3D12_INDEX_BUFFER_VIEW mCurrIndexBufferView;
    const Buffer* mBoundIndexBuffer;

    D3D12_VERTEX_BUFFER_VIEW mCurrVertexBufferViews[NFE_RENDERER_MAX_VERTEX_BUFFERS];
    const Buffer* mBoundVertexBuffers[NFE_RENDERER_MAX_VERTEX_BUFFERS];
    uint8 mNumBoundVertexBuffers;
};

} // namespace Renderer
} // namespace NFE

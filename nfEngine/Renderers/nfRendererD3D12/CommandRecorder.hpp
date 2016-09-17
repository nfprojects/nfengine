/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's command recorder.
 */

#pragma once

#include "../RendererInterface/CommandRecorder.hpp"
#include "Common.hpp"
#include "RingBuffer.hpp"
#include "ResourceBinding.hpp"
#include "CommandList.hpp"
#include "ResourceStateCache.hpp"

#include "nfCommon/Containers/HashSet.hpp"
#include "nfCommon/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

// predeclarations
class RenderTarget;
class PipelineState;
class ComputePipelineState;
class Buffer;
class CommandList;

/**
 * Helper structure used for tracking resources references (Direct3D 12 does not do it by its own).
 */
class ReferencedResources
{
    // lists of referenced resources by this command list
    Common::HashSet<RenderTargetPtr> renderTargets;
    Common::HashSet<ResourceBindingLayoutPtr> resBindingLayouts;
    Common::HashSet<ResourceBindingInstancePtr> resBindingInstances;
    Common::HashSet<BufferPtr> buffers;
    Common::HashSet<PipelineStatePtr> pipelineStates;
    Common::HashSet<ComputePipelineStatePtr> computePipelineStates;
};

class CommandRecorder : public ICommandRecorder
{
private:
    uint64 mFrameCounter;       // total frame counter
    uint32 mFrameCount;         // max number of queued frames
    uint32 mFrameBufferIndex;   // current frame (command allocator index)
    Common::DynArray<D3DPtr<ID3D12CommandAllocator>> mCommandAllocators; // command allocator for each frame
    Common::DynArray<ReferencedResources> mReferencedResources; // referenced resources by each frame

    CommandList* mCommandListObject;
    ID3D12GraphicsCommandList* mCommandList; // cached D3D12 command list pointer

    ResourceStateCache mResourceStateCache;

    // ring buffer for dynamic buffers support
    RingBuffer mRingBuffer;
    const Buffer* mBoundVolatileCBuffers[NFE_RENDERER_MAX_VOLATILE_CBUFFERS];
    const Buffer* mBoundComputeVolatileCBuffers[NFE_RENDERER_MAX_VOLATILE_CBUFFERS];

    RenderTarget* mCurrRenderTarget;
    ResourceBindingLayout* mBindingLayout;
    ResourceBindingLayout* mCurrBindingLayout;
    PipelineState* mCurrPipelineState;
    PipelineState* mPipelineState;

    ResourceBindingLayout* mComputeBindingLayout;
    ComputePipelineState* mCurrComputePipelineState;

    D3D12_PRIMITIVE_TOPOLOGY mCurrPrimitiveTopology;

    D3D12_VERTEX_BUFFER_VIEW mCurrVertexBufferViews[NFE_RENDERER_MAX_VERTEX_BUFFERS];
    const Buffer* mBoundVertexBuffers[NFE_RENDERER_MAX_VERTEX_BUFFERS];
    uint32 mNumBoundVertexBuffers;

    void UpdateStates();
    void UnsetRenderTarget();

    void WriteDynamicBuffer(Buffer* buffer, size_t offset, size_t size, const void* data);
    void WriteVolatileBuffer(Buffer* buffer, const void* data);

public:
    CommandRecorder();
    ~CommandRecorder();
    bool Init(ID3D12Device* device, uint32 frameCount);

    /// Common methods
    bool Begin() override;
    void* MapBuffer(const BufferPtr& buffer, MapType type) override;
    void UnmapBuffer(const BufferPtr& buffer) override;
    bool WriteBuffer(const BufferPtr& buffer, size_t offset, size_t size, const void* data) override;
    void CopyTexture(const TexturePtr& src, const TexturePtr& dest) override;
    CommandListID Finish() override;

    /// Compute pipeline methods
    void SetVertexBuffers(int num, const BufferPtr* vertexBuffers, int* strides, int* offsets) override;
    void SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format) override;
    void BindResources(size_t slot, const ResourceBindingInstancePtr& bindingSetInstance) override;
    void BindVolatileCBuffer(size_t slot, const BufferPtr& buffer) override;
    void SetRenderTarget(const RenderTargetPtr& renderTarget) override;
    void SetResourceBindingLayout(const ResourceBindingLayoutPtr& layout) override;
    void SetPipelineState(const PipelineStatePtr& state) override;
    void SetStencilRef(unsigned char ref) override;
    void SetViewport(float left, float width, float top, float height, float minDepth, float maxDepth) override;
    void SetScissors(int left, int top, int right, int bottom) override;
    void Clear(int flags, uint32 numTargets, const uint32* slots, const Math::Float4* colors, float depthValue, uint8 stencilValue) override;
    void Draw(int vertexNum, int instancesNum, int vertexOffset, int instanceOffset) override;
    void DrawIndexed(int indexNum, int instancesNum, int indexOffset, int vertexOffset, int instanceOffset) override;

    /// Compute pipeline methods
    void BindComputeResources(size_t slot, const ResourceBindingInstancePtr& bindingSetInstance) override;
    void BindComputeVolatileCBuffer(size_t slot, const BufferPtr& buffer) override;
    void SetComputeResourceBindingLayout(const ResourceBindingLayoutPtr& layout) override;
    void SetComputePipelineState(const ComputePipelineStatePtr& state) override;
    void Dispatch(uint32 x, uint32 y, uint32 z) override;

    /// Debugging
    void BeginDebugGroup(const char* text) override;
    void EndDebugGroup() override;
    void InsertDebugMarker(const char* text) override;

    bool OnFinishFrame(uint64 fenceValue);
    bool OnFrameCompleted(uint64 fenceValue);
};

} // namespace Renderer
} // namespace NFE

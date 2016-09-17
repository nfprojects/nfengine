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
#include "Resource.hpp"
#include "ReferencedResourcesList.hpp"

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


class CommandRecorder : public ICommandRecorder
{
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
    void SetVertexBuffers(uint32 num, const BufferPtr* vertexBuffers, uint32* strides, uint32* offsets) override;
    void SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format) override;
    void BindResources(uint32 slot, const ResourceBindingInstancePtr& bindingSetInstance) override;
    void BindVolatileCBuffer(uint32 slot, const BufferPtr& buffer) override;
    void SetRenderTarget(const RenderTargetPtr& renderTarget) override;
    void SetResourceBindingLayout(const ResourceBindingLayoutPtr& layout) override;
    void SetPipelineState(const PipelineStatePtr& state) override;
    void SetStencilRef(uint8 ref) override;
    void SetViewport(float left, float width, float top, float height, float minDepth, float maxDepth) override;
    void SetScissors(int32 left, int32 top, int32 right, int32 bottom) override;
    void Clear(uint32 flags, uint32 numTargets, const uint32* slots, const Math::Float4* colors, float depthValue, uint8 stencilValue) override;
    void Draw(uint32 vertexNum, uint32 instancesNum, uint32 vertexOffset, uint32 instanceOffset) override;
    void DrawIndexed(uint32 indexNum, uint32 instancesNum, uint32 indexOffset, int32 vertexOffset, uint32 instanceOffset) override;

    /// Compute pipeline methods
    void BindComputeResources(uint32 slot, const ResourceBindingInstancePtr& bindingSetInstance) override;
    void BindComputeVolatileCBuffer(uint32 slot, const BufferPtr& buffer) override;
    void SetComputeResourceBindingLayout(const ResourceBindingLayoutPtr& layout) override;
    void SetComputePipelineState(const ComputePipelineStatePtr& state) override;
    void Dispatch(uint32 x, uint32 y, uint32 z) override;

    /// Debugging
    void BeginDebugGroup(const char* text) override;
    void EndDebugGroup() override;
    void InsertDebugMarker(const char* text) override;

    bool CanBeDeleted() const;
    bool OnFinishFrame(uint64 frameIndex);
    bool OnFrameCompleted(uint64 frameIndex, uint32 frameBufferIndex);

private:
    void Internal_UpdateStates();
    void Internal_UnsetRenderTarget();

    void Internal_WriteDynamicBuffer(Buffer* buffer, size_t offset, size_t size, const void* data);
    void Internal_WriteVolatileBuffer(Buffer* buffer, const void* data);

    ReferencedResourcesList& Internal_GetReferencedResources();

    uint64 mLastFinishedFrameIndex;
    uint64 mLastCompletedFrameIndex;
    uint32 mFrameBufferIndex;   // current frame (command allocator index)

    // TODO move to device?
    Common::DynArray<D3DPtr<ID3D12CommandAllocator>> mCommandAllocators; // command allocator for each frame
    Common::DynArray<ReferencedResourcesList> mReferencedResources; // referenced resources by each frame

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
};

} // namespace Renderer
} // namespace NFE

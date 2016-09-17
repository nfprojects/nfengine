/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's command buffer.
 */

#pragma once

#include "../RendererInterface/CommandBuffer.hpp"
#include "Common.hpp"
#include "RingBuffer.hpp"
#include "ResourceBinding.hpp"
#include "CommandList.hpp"

namespace NFE {
namespace Renderer {

// predeclarations
class RenderTarget;
class PipelineState;
class ComputePipelineState;
class Buffer;

class CommandBuffer : public ICommandBuffer
{
    CommandListPtr mCommandListPtr;
    ID3D12GraphicsCommandList* mCommandList;

    // ring buffer for dynamic buffers support
    RingBuffer mRingBuffer;
    Buffer* mBoundVolatileCBuffers[NFE_RENDERER_MAX_VOLATILE_CBUFFERS];
    Buffer* mBoundComputeVolatileCBuffers[NFE_RENDERER_MAX_VOLATILE_CBUFFERS];

    RenderTarget* mCurrRenderTarget;
    ResourceBindingLayout* mBindingLayout;
    ResourceBindingLayout* mCurrBindingLayout;
    PipelineState* mCurrPipelineState;
    PipelineState* mPipelineState;

    ResourceBindingLayout* mComputeBindingLayout;
    ComputePipelineState* mCurrComputePipelineState;

    D3D12_PRIMITIVE_TOPOLOGY mCurrPrimitiveTopology;

    D3D12_VERTEX_BUFFER_VIEW mCurrVertexBufferViews[NFE_RENDERER_MAX_VERTEX_BUFFERS];
    Buffer* mBoundVertexBuffers[NFE_RENDERER_MAX_VERTEX_BUFFERS];
    uint32 mNumBoundVertexBuffers;

    void UpdateStates();
    void UnsetRenderTarget();

    void WriteDynamicBuffer(Buffer* buffer, size_t offset, size_t size, const void* data);
    void WriteVolatileBuffer(Buffer* buffer, const void* data);

public:
    CommandBuffer();
    ~CommandBuffer();
    bool Init(ID3D12Device* device);

    /// Common methods
    void Reset() override;
    void* MapBuffer(IBuffer* buffer, MapType type) override;
    void UnmapBuffer(IBuffer* buffer) override;
    bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data) override;
    void CopyTexture(ITexture* src, ITexture* dest) override;
    std::unique_ptr<ICommandList> Finish() override;

    /// Compute pipeline methods
    void SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets) override;
    void SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format) override;
    void BindResources(size_t slot, IResourceBindingInstance* bindingSetInstance) override;
    void BindVolatileCBuffer(size_t slot, IBuffer* buffer) override;
    void SetRenderTarget(IRenderTarget* renderTarget) override;
    void SetResourceBindingLayout(IResourceBindingLayout* layout) override;
    void SetPipelineState(IPipelineState* state) override;
    void SetStencilRef(unsigned char ref) override;
    void SetViewport(float left, float width, float top, float height,
                     float minDepth, float maxDepth) override;
    void SetScissors(int left, int top, int right, int bottom) override;
    void Clear(int flags, uint32 numTargets, const uint32* slots, const Math::Float4* colors,
               float depthValue, uint8 stencilValue) override;
    void Draw(int vertexNum, int instancesNum, int vertexOffset,
              int instanceOffset) override;
    void DrawIndexed(int indexNum, int instancesNum, int indexOffset,
                     int vertexOffset, int instanceOffset) override;

    /// Compute pipeline methods
    void BindComputeResources(size_t slot, IResourceBindingInstance* bindingSetInstance) override;
    void BindComputeVolatileCBuffer(size_t slot, IBuffer* buffer) override;
    void SetComputeResourceBindingLayout(IResourceBindingLayout* layout) override;
    void SetComputePipelineState(IComputePipelineState* state) override;
    void Dispatch(uint32 x, uint32 y, uint32 z) override;

    /// Debugging
    void BeginDebugGroup(const char* text) override;
    void EndDebugGroup() override;
    void InsertDebugMarker(const char* text) override;
};

} // namespace Renderer
} // namespace NFE

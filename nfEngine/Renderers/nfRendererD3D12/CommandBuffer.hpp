/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's command buffer.
 */

#pragma once

#include "../RendererInterface/CommandBuffer.hpp"
#include "Common.hpp"
#include "RenderTarget.hpp"
#include "Shader.hpp"
#include "PipelineState.hpp"
#include "ResourceBinding.hpp"
#include "RingBuffer.hpp"
#include "Buffer.hpp"


namespace NFE {
namespace Renderer {

struct CommandList : public ICommandList
{
    // TODO what about generating multiple command list on a single command buffer?
    CommandBuffer* commandBuffer;
};

class CommandBuffer : public ICommandBuffer
{
    friend class Device;

    uint64 mFrameCounter;       // total frame counter
    uint32 mFrameCount;         // number of queued frames
    uint32 mFrameBufferIndex;   // current frame (command allocator index)
    std::vector<D3DPtr<ID3D12CommandAllocator>> mCommandAllocators;
    D3DPtr<ID3D12GraphicsCommandList> mCommandList;

    // synchronization objects
    D3DPtr<ID3D12Fence> mFence;
    HANDLE mFenceEvent;
    std::vector<uint64> mFenceValues;

    // ring buffer for dynamic buffers support
    RingBuffer mRingBuffer;
    Buffer* mBoundVolatileCBuffers[NFE_RENDERER_MAX_VOLATILE_CBUFFERS];

    RenderTarget* mCurrRenderTarget;
    ResourceBindingLayout* mBindingLayout;
    ResourceBindingLayout* mCurrBindingLayout;
    PipelineState* mCurrPipelineState;
    PipelineState* mPipelineState;

    D3D12_PRIMITIVE_TOPOLOGY mCurrPrimitiveTopology;

    D3D12_VERTEX_BUFFER_VIEW mCurrVertexBufferViews[NFE_RENDERER_MAX_VERTEX_BUFFERS];
    Buffer* mBoundVertexBuffers[NFE_RENDERER_MAX_VERTEX_BUFFERS];
    uint32 mNumBoundVertexBuffers;

    // is in reset state?
    bool mReset;

    void UpdateStates();
    void UnsetRenderTarget();

    // called by Device, when command list was queued
    bool MoveToNextFrame(ID3D12CommandQueue* commandQueue);

    void WriteDynamicBuffer(Buffer* buffer, size_t offset, size_t size, const void* data);
    void WriteVolatileBuffer(Buffer* buffer, const void* data);

public:
    CommandBuffer();
    ~CommandBuffer();
    bool Init(ID3D12Device* device);

    /// Shader resources setup methods

    void Reset() override;
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

    /// "Executive" methods

    void* MapBuffer(IBuffer* buffer, MapType type) override;
    void UnmapBuffer(IBuffer* buffer) override;
    bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data) override;
    void CopyTexture(ITexture* src, ITexture* dest) override;
    void Clear(int flags, uint32 numTargets, const uint32* slots, const Math::Float4* colors,
               float depthValue, uint8 stencilValue) override;
    void Draw(int vertexNum, int instancesNum, int vertexOffset,
              int instanceOffset) override;
    void DrawIndexed(int indexNum, int instancesNum, int indexOffset,
                     int vertexOffset, int instanceOffset) override;

    std::unique_ptr<ICommandList> Finish() override;

    /// Debugging

    void BeginDebugGroup(const char* text) override;
    void EndDebugGroup() override;
    void InsertDebugMarker(const char* text) override;
};

} // namespace Renderer
} // namespace NFE

/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Command Buffer
 */

#pragma once

#include "../RendererInterface/CommandBuffer.hpp"

#include "Defines.hpp"
#include "RenderTarget.hpp"
#include "PipelineState.hpp"
#include "Buffer.hpp"
#include "RingBuffer.hpp"


namespace NFE {
namespace Renderer {

class CommandBuffer;

struct CommandList : public ICommandList
{
    CommandBuffer* cmdBuffer;
};

class CommandBuffer : public ICommandBuffer
{
    friend class Device;

    VkCommandBuffer mCommandBuffer;
    VkCommandBufferBeginInfo mCommandBufferBeginInfo;
    RenderTarget* mRenderTarget;
    bool mActiveRenderPass;
    ResourceBindingLayout* mResourceBindingLayout;
    RingBuffer mRingBuffer;
    VkFence mFences[VK_FENCE_COUNT]; // TODO TEMPORARY
    uint32 mCurrentFence;
    Buffer* mBoundVolatileBuffers[VK_MAX_VOLATILE_BUFFERS];

    bool WriteDynamicBuffer(Buffer* b, size_t offset, size_t size, const void* data);
    bool WriteVolatileBuffer(Buffer* b, size_t size, const void* data);

public:
    CommandBuffer();
    ~CommandBuffer();

    bool Init();

    /// Common methods
    void Reset() override;
    void* MapBuffer(IBuffer* buffer, MapType type) override;
    void UnmapBuffer(IBuffer* buffer) override;
    bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data) override;
    void CopyTexture(ITexture* src, ITexture* dest) override;
    std::unique_ptr<ICommandList> Finish() override;

    /// Graphics pipeline methods
    void SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets) override;
    void SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format) override;
    void BindResources(size_t slot, IResourceBindingInstance* bindingSetInstance) override;
    void BindVolatileCBuffer(size_t slot, IBuffer* buffer) override;
    void SetResourceBindingLayout(IResourceBindingLayout* layout) override;
    void SetRenderTarget(IRenderTarget* renderTarget) override;
    void SetPipelineState(IPipelineState* state) override;
    void SetStencilRef(unsigned char ref) override;
    void SetViewport(float left, float width, float top, float height,
                     float minDepth, float maxDepth) override;
    void SetScissors(int left, int top, int right, int bottom) override;
    void Clear(int flags, uint32 numTargets, const uint32* slots, const Math::Float4* colors,
               float depthValue, uint8 stencilValue) override;
    void Draw(int vertexNum, int instancesNum, int vertexOffset, int instanceOffset) override;
    void DrawIndexed(int indexNum, int instancesNum, int indexOffset, int vertexOffset, int instanceOffset) override;

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

    // TODO COMPLETELY TEMPORARY
    void AdvanceFrame();
};

} // namespace Renderer
} // namespace NFE

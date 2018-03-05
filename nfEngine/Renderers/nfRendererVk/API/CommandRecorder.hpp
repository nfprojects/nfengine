/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Command Recorder
 */

#pragma once

#include "../RendererInterface/CommandRecorder.hpp"

#include "Defines.hpp"
#include "RenderTarget.hpp"
#include "PipelineState.hpp"
#include "Buffer.hpp"
#include "Internal/RingBuffer.hpp"

#include "nfCommon/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

class CommandRecorder: public ICommandRecorder
{
    friend class Device;

    DevicePtr mDevicePtr;

    VkCommandBuffer mCommandBuffer;
    VkCommandBufferBeginInfo mCommandBufferBeginInfo;
    RenderTarget* mRenderTarget;
    bool mActiveRenderPass;
    ResourceBindingLayout* mResourceBindingLayout;
    Buffer* mBoundVolatileBuffers[VK_MAX_VOLATILE_BUFFERS];

    bool WriteDynamicBuffer(Buffer* b, size_t offset, size_t size, const void* data);
    bool WriteVolatileBuffer(Buffer* b, size_t size, const void* data);

public:
    CommandRecorder();
    ~CommandRecorder();

    bool Init(DevicePtr& device);

    /// Common methods
    bool Begin() override;
    void* MapBuffer(const BufferPtr& buffer, MapType type) override;
    void UnmapBuffer(const BufferPtr& buffer) override;
    bool WriteBuffer(const BufferPtr& buffer, size_t offset, size_t size, const void* data) override;
    void CopyTexture(const TexturePtr& src, const TexturePtr& dest) override;
    CommandListID Finish() override;

    /// Graphics pipeline methods
    void SetVertexBuffers(uint32 num, const BufferPtr* vertexBuffers, uint32* strides, uint32* offsets) override;
    void SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format) override;
    void BindResources(uint32 slot, const ResourceBindingInstancePtr& bindingSetInstance) override;
    void BindVolatileCBuffer(uint32 slot, const BufferPtr& buffer) override;
    void SetResourceBindingLayout(const ResourceBindingLayoutPtr& layout) override;
    void SetRenderTarget(const RenderTargetPtr& renderTarget) override;
    void SetPipelineState(const PipelineStatePtr& state) override;
    void SetStencilRef(unsigned char ref) override;
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
};

} // namespace Renderer
} // namespace NFE

/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Command Buffer
 */

#pragma once

#include "../RendererInterface/CommandBuffer.hpp"

namespace NFE {
namespace Renderer {

class CommandBuffer : public ICommandBuffer
{
    friend class Device;

public:
    CommandBuffer();
    ~CommandBuffer();

    /// Resources setup methods
    void Reset() override;
    void SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets) override;
    void SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format) override;
    void BindResources(size_t slot, IResourceBindingInstance* bindingSetInstance) override;
    void SetResourceBindingLayout(IResourceBindingLayout* layout) override;
    void SetRenderTarget(IRenderTarget* renderTarget) override;
    void SetShaderProgram(IShaderProgram* shaderProgram) override;
    void SetPipelineState(IPipelineState* state) override;
    void SetStencilRef(unsigned char ref) override;
    void SetViewport(float left, float width, float top, float height,
                     float minDepth, float maxDepth) override;
    void SetScissors(int left, int top, int right, int bottom) override;

    /// Executives
    void* MapBuffer(IBuffer* buffer, MapType type) override;
    void UnmapBuffer(IBuffer* buffer) override;
    bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data) override;
    void CopyTexture(ITexture* src, ITexture* dest) override;
    void Clear(int flags, const float* color, float depthValue, unsigned char stencilValue) override;
    void Draw(int vertexNum, int instancesNum, int vertexOffset, int instanceOffset) override;
    void DrawIndexed(int indexNum, int instancesNum, int indexOffset, int vertexOffset, int instanceOffset) override;
    std::unique_ptr<ICommandList> Finish() override;

    /// Debugging
    void BeginDebugGroup(const char* text) override;
    void EndDebugGroup() override;
    void InsertDebugMarker(const char* text) override;
};

} // namespace Renderer
} // namespace NFE

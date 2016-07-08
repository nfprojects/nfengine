/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan Command Buffer
 */

#pragma once

#include "../RendererInterface/CommandBuffer.hpp"

#include "Defines.hpp"

namespace NFE {
namespace Renderer {

class CommandBuffer : public ICommandBuffer
{
    friend class Device;

    VkCommandBuffer mVkCommandBuffer;

public:
    CommandBuffer();
    ~CommandBuffer();

    bool Init();

    /// Resources setup methods
    void Reset();
    void SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets);
    void SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format);
    void BindResources(size_t slot, IResourceBindingInstance* bindingSetInstance);
    void SetResourceBindingLayout(IResourceBindingLayout* layout);
    void SetRenderTarget(IRenderTarget* renderTarget);
    void SetShaderProgram(IShaderProgram* shaderProgram);
    void SetPipelineState(IPipelineState* state);
    void SetStencilRef(unsigned char ref);
    void SetViewport(float left, float width, float top, float height,
                     float minDepth, float maxDepth);
    void SetScissors(int left, int top, int right, int bottom);

    /// Executives
    void* MapBuffer(IBuffer* buffer, MapType type);
    void UnmapBuffer(IBuffer* buffer);
    bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data);
    void CopyTexture(ITexture* src, ITexture* dest);
    void Clear(int flags, const float* color, float depthValue, unsigned char stencilValue);
    void Draw(PrimitiveType type, int vertexNum, int instancesNum, int vertexOffset,
              int instanceOffset);
    void DrawIndexed(PrimitiveType type, int indexNum, int instancesNum, int indexOffset,
                     int vertexOffset, int instanceOffset);
    std::unique_ptr<ICommandList> Finish();

    /// Debugging
    void BeginDebugGroup(const char* text);
    void EndDebugGroup();
    void InsertDebugMarker(const char* text);
};

} // namespace Renderer
} // namespace NFE

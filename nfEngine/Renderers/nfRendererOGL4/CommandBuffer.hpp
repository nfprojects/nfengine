/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of OpenGL 4 Command Buffer
 */

#pragma once

#include "../RendererInterface/CommandBuffer.hpp"

#include "RenderTarget.hpp"

namespace NFE {
namespace Renderer {

class CommandBuffer : public ICommandBuffer
{
    friend class Device;

    PrimitiveType mCurrentPrimitiveType;
    RenderTarget* mCurrentRenderTarget;

    ShaderProgramDesc mBoundShaders;

public:
    CommandBuffer();
    ~CommandBuffer();

    /// Resources setup methods
    void SetVertexLayout(IVertexLayout* vertexLayout);
    void SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets);
    void SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format);
    void SetSamplers(ISampler** samplers, int num, ShaderType target);
    void SetTextures(ITexture** textures, int num, ShaderType target);
    void SetConstantBuffers(IBuffer** constantBuffers, int num, ShaderType target);
    void SetRenderTarget(IRenderTarget* renderTarget);
    void SetShaderProgram(IShaderProgram* shaderProgram);
    void SetShader(IShader* shader);
    void SetBlendState(IBlendState* state);
    void SetRasterizerState(IRasterizerState* state);
    void SetDepthState(IDepthState* state);
    void SetViewport(float left, float width, float top, float height,
                     float minDepth, float maxDepth);

    /// Executives
    bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data);
    bool ReadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data);
    void CopyTexture(ITexture* src, ITexture* dest);
    bool ReadTexture(ITexture* tex, void* data);
    void Clear(int flags, const float* color, float depthValue);
    void Draw(PrimitiveType type, int vertexNum, int instancesNum, int vertexOffset,
              int instanceOffset);
    void DrawIndexed(PrimitiveType type, int indexNum, int instancesNum, int indexOffset,
                     int vertexOffset, int instanceOffset);
    void Execute(ICommandBuffer* commandBuffer, bool saveState);
};

} // namespace Renderer
} // namespace NFE

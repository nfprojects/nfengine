/**
 * @file    CommandBuffer.hpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "PipelineState.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "RenderTarget.hpp"

namespace NFE {
namespace Renderer {

/**
 * Interface allowing to control rendering pipeline state and executing rendering commands.
 */
class ICommandBuffer
{
public:
    virtual ~ICommandBuffer() {}

    /// Shader resources setup methods

    virtual void SetVertexLayout(IVertexLayout* vertexLayout) = 0;
    virtual void SetVertexBuffers(int num, IBuffer** vertexBuffers,
                                  int* strides, int* offsets) = 0;
    virtual void SetIndexBuffer(IBuffer* indexBuffer) = 0;
    virtual void SetSamplers(ISampler** samplers, int num, ShaderType target) = 0;
    virtual void SetTextures(ITexture** textures, int num, ShaderType target) = 0;
    virtual void SetConstantBuffers(IBuffer** constantBuffers, int num, ShaderType target) = 0;
    virtual void SetRenderTargets(IRenderTarget** renderTargets, int num) = 0;
    virtual void SetShaderProgram(IShaderProgram* shaderProgram) = 0;
    virtual void SetBlendState(IBlendState* state) = 0;
    virtual void SetRasterizerState(IRasterizerState* state) = 0;
    virtual void SetDepthState(IDepthState* state) = 0;
    virtual void SetViewport(float left, float width, float top, float height,
                             float minDepth, float maxDepth) = 0;

    /// "Executive" methods

    virtual void CopyTexture(ITexture* src, ITexture* dest) = 0;
    virtual void Clear(const float* color) = 0;
    virtual void Draw(PrimitiveType type, int vertexNum,
                      int instancesNum = 1, int indexOffset = 0,
                      int vertexOffset = 0, int instanceOffset = 0) = 0;

    virtual void Execute(ICommandBuffer* commandBuffer, bool saveState) = 0;
};

} // namespace Renderer
} // namespace NFE

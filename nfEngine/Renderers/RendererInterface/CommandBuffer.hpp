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
#include "Backbuffer.hpp"
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
    virtual void SetRenderTarget(IRenderTarget* renderTarget) = 0;
    virtual void SetShaderProgram(IShaderProgram* shaderProgram) = 0;
    virtual void SetBlendState(IBlendState* state) = 0;
    virtual void SetRasterizerState(IRasterizerState* state) = 0;
    virtual void SetDepthState(IDepthState* state) = 0;
    virtual void SetViewport(float left, float width, float top, float height,
                             float minDepth, float maxDepth) = 0;

    /// "Executive" methods

    /**
     * Write data from the CPU memory to a GPU buffer.
     * @param buffer Target buffer.
     * @param offset Offset in the GPU buffer (in bytes).
     * @param size   Number of bytes to write.
     * @param data   Pointer to source CPU buffer.
     * @return true on success.
     */
    virtual bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data) = 0;

    /**
     * Read data from a GPU buffer to the CPU memory.
     * @param buffer Source buffer.
     * @param offset Offset in the GPU buffer (in bytes).
     * @param size   Number of bytes to read.
     * @param data   Pointer to target CPU buffer.
     * @return true on success.
     */
    virtual bool ReadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data) = 0;

    /**
     * Copy contents of texture @p src to @p dest.
     * The textures source and destination texture must be the same type, size, format
     * and sample count.
     */
    virtual void CopyTexture(ITexture* src, ITexture* dest) = 0;

    /**
     * Read texture content to a CPU buffer.
     * @param tex Texture to read.
     * @praram data Target CPU buffer.
     *
     * TODO: selecting mipmap level, layer/slice number, etc.
     */
    virtual bool ReadTexture(ITexture* tex, void* data) = 0;

    /**
     * Clear bound render targets with a color.
     * @param color 4 element array of floats.
     *
     * TODO: Depth buffer clearing, MRT support.
     */
    virtual void Clear(const float* color) = 0;

    /**
     * Draw geometry.
     * @param type         Primitives type.
     * @param vertexNum    Vertices number per instance.
     * @param instancesNum Number of instances to draw.
     * @indexOffset        Location of the first index
     * @vertexOffset       Vertex buffer offset (in elements).
     * @instanceOffset     Per-instance buffer offset (in elements).
     */
    virtual void Draw(PrimitiveType type, int vertexNum,
                      int instancesNum = 1, int indexOffset = 0,
                      int vertexOffset = 0, int instanceOffset = 0) = 0;

    /**
     * Execute a command buffer.
     * @param saveState Save previous pipeline state after finishing executing command buffer.
     */
    virtual void Execute(ICommandBuffer* commandBuffer, bool saveState) = 0;
};

} // namespace Renderer
} // namespace NFE

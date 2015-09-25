/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's command buffer
 */

// TODO:
// 1. Improve logging, but be careful - functions from this source file will be called thousands
//    times per frame. Too much messages could flood a logger output.

#include "PCH.hpp"
#include "CommandBuffer.hpp"
#include "RendererD3D12.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "RenderTarget.hpp"
#include "PipelineState.hpp"
#include "Sampler.hpp"
#include "Translations.hpp"
#include "../../nfCommon/Logger.hpp"
#include "../../nfCommon/Win/Common.hpp"  // required for ID3DUserDefinedAnnotation

namespace NFE {
namespace Renderer {

CommandBuffer::CommandBuffer()
{
}

CommandBuffer::~CommandBuffer()
{
}

void CommandBuffer::Reset()
{
}

void CommandBuffer::SetViewport(float left, float width, float top, float height,
                                float minDepth, float maxDepth)
{
    UNUSED(left);
    UNUSED(width);
    UNUSED(top);
    UNUSED(height);
    UNUSED(minDepth);
    UNUSED(maxDepth);
}

void CommandBuffer::SetVertexLayout(IVertexLayout* vertexLayout)
{
    UNUSED(vertexLayout);
}

void CommandBuffer::SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets)
{
    UNUSED(num);
    UNUSED(vertexBuffers);
    UNUSED(strides);
    UNUSED(offsets);
}

void CommandBuffer::SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format)
{
    UNUSED(indexBuffer);
    UNUSED(format);
}

void CommandBuffer::SetSamplers(ISampler** samplers, int num, ShaderType target, int slotOffset)
{
    UNUSED(samplers);
    UNUSED(num);
    UNUSED(target);
    UNUSED(slotOffset);
}

void CommandBuffer::SetTextures(ITexture** textures, int num, ShaderType target, int slotOffset)
{
    UNUSED(textures);
    UNUSED(num);
    UNUSED(target);
    UNUSED(slotOffset);
}

void CommandBuffer::SetConstantBuffers(IBuffer** constantBuffers, int num, ShaderType target,
                                       int slotOffset)
{
    UNUSED(constantBuffers);
    UNUSED(num);
    UNUSED(target);
    UNUSED(slotOffset);
}

void CommandBuffer::SetRenderTarget(IRenderTarget* renderTarget)
{
    UNUSED(renderTarget);
}

void CommandBuffer::SetShaderProgram(IShaderProgram* shaderProgram)
{
    UNUSED(shaderProgram);
}

void CommandBuffer::SetShader(IShader* shader)
{
    UNUSED(shader);
}

void CommandBuffer::SetBlendState(IBlendState* state)
{
    UNUSED(state);
}

void CommandBuffer::SetRasterizerState(IRasterizerState* state)
{
    UNUSED(state);
}

void CommandBuffer::SetDepthState(IDepthState* state)
{
    UNUSED(state);
}

bool CommandBuffer::WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data)
{
    UNUSED(buffer);
    UNUSED(offset);
    UNUSED(size);
    UNUSED(data);
    return false;
}

bool CommandBuffer::ReadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data)
{
    UNUSED(buffer);
    UNUSED(offset);
    UNUSED(size);
    UNUSED(data);
    return false;
}

void CommandBuffer::CopyTexture(ITexture* src, ITexture* dest)
{
    UNUSED(src);
    UNUSED(dest);
}

bool CommandBuffer::ReadTexture(ITexture* tex, void* data)
{
    UNUSED(tex);
    UNUSED(data);
    return false;
}

void CommandBuffer::Clear(int flags, const float* color, float depthValue)
{
    UNUSED(flags);
    UNUSED(color);
    UNUSED(depthValue);
}

void CommandBuffer::Draw(PrimitiveType type, int vertexNum, int instancesNum, int vertexOffset,
                         int instanceOffset)
{
    UNUSED(type);
    UNUSED(vertexNum);
    UNUSED(instancesNum);
    UNUSED(vertexOffset);
    UNUSED(instanceOffset);
}

void CommandBuffer::DrawIndexed(PrimitiveType type, int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    UNUSED(type);
    UNUSED(indexNum);
    UNUSED(instancesNum);
    UNUSED(indexOffset);
    UNUSED(vertexOffset);
    UNUSED(instanceOffset);
}

void CommandBuffer::Execute(ICommandBuffer* commandBuffer, bool saveState)
{
    UNUSED(commandBuffer);
    UNUSED(saveState);
}

void CommandBuffer::BeginDebugGroup(const char* text)
{
    UNUSED(text);
}

void CommandBuffer::EndDebugGroup()
{
}

void CommandBuffer::InsertDebugMarker(const char* text)
{
    UNUSED(text);
}

} // namespace Renderer
} // namespace NFE

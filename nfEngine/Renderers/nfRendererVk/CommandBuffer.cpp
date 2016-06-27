/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Command Buffer
 */

#include "PCH.hpp"

#include "Defines.hpp"

#include "CommandBuffer.hpp"


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

void CommandBuffer::BindResources(size_t slot, IResourceBindingInstance* bindingSetInstance)
{
    UNUSED(slot);
    UNUSED(bindingSetInstance);
}

void CommandBuffer::SetResourceBindingLayout(IResourceBindingLayout* layout)
{
    UNUSED(layout);
}

void CommandBuffer::SetRenderTarget(IRenderTarget* renderTarget)
{
    UNUSED(renderTarget);
}

void CommandBuffer::SetShaderProgram(IShaderProgram* shaderProgram)
{
    UNUSED(shaderProgram);
}

void CommandBuffer::SetPipelineState(IPipelineState* state)
{
    UNUSED(state);
}

void CommandBuffer::SetStencilRef(unsigned char ref)
{
    UNUSED(ref);
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

void CommandBuffer::SetScissors(int left, int top, int right, int bottom)
{
    UNUSED(left);
    UNUSED(top);
    UNUSED(right);
    UNUSED(bottom);
}

void* CommandBuffer::MapBuffer(IBuffer* buffer, MapType type)
{
    UNUSED(buffer);
    UNUSED(type);
    return nullptr;
}

void CommandBuffer::UnmapBuffer(IBuffer* buffer)
{
    UNUSED(buffer);
}

bool CommandBuffer::WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data)
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

void CommandBuffer::Clear(int flags, const float* color, float depthValue,
                          unsigned char stencilValue)
{
    UNUSED(flags);
    UNUSED(color);
    UNUSED(depthValue);
    UNUSED(stencilValue);
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

std::unique_ptr<ICommandList> CommandBuffer::Finish()
{
    return nullptr;
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

/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan Command Buffer
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Device.hpp"
#include "Translations.hpp"
#include "CommandBuffer.hpp"

#include <string.h>


namespace NFE {
namespace Renderer {


CommandBuffer::CommandBuffer()
    : mCommandBuffer(VK_NULL_HANDLE)
{
}

CommandBuffer::~CommandBuffer()
{
    if (mCommandBuffer)
        vkFreeCommandBuffers(gDevice->GetDevice(), gDevice->GetCommandPool(),
                                               1, &mCommandBuffer);
}

bool CommandBuffer::Init()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    VK_ZERO_MEMORY(allocInfo);
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = gDevice->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    VkResult result = vkAllocateCommandBuffers(gDevice->GetDevice(), &allocInfo, &mCommandBuffer);
    CHECK_VKRESULT(result, "Failed to allocate a command buffer");

    VK_ZERO_MEMORY(mCommandBufferBeginInfo);
    mCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    mCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    LOG_INFO("Command Buffer initialized successfully.");
    return true;
}

void CommandBuffer::Reset()
{
    vkBeginCommandBuffer(mCommandBuffer, &mCommandBufferBeginInfo);
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
    VkViewport viewport;
    memset(&viewport, 0, sizeof(viewport));
    viewport.x = left;
    viewport.y = top;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
}

void CommandBuffer::SetScissors(int left, int top, int right, int bottom)
{
    VkRect2D scissor;
    scissor.offset.x = left;
    scissor.offset.y = top;
    scissor.extent.width = right - left;
    scissor.extent.height = bottom - top;
    vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
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


void CommandBuffer::Draw(int vertexNum, int instancesNum, int vertexOffset,
                         int instanceOffset)
{
    UNUSED(vertexNum);
    UNUSED(instancesNum);
    UNUSED(vertexOffset);
    UNUSED(instanceOffset);
}

void CommandBuffer::DrawIndexed(int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    UNUSED(indexNum);
    UNUSED(instancesNum);
    UNUSED(indexOffset);
    UNUSED(vertexOffset);
    UNUSED(instanceOffset);
}

std::unique_ptr<ICommandList> CommandBuffer::Finish()
{
    VkResult result = vkEndCommandBuffer(mCommandBuffer);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Error during Constant Buffer recording: %d (%s)",
                  result, TranslateVkResultToString(result));
        return nullptr;
    }

    std::unique_ptr<CommandList> list(new (std::nothrow) CommandList);
    if (!list)
    {
        LOG_ERROR("Failed to allocate memory for command list");
        return nullptr;
    }

    list->cmdBuffer = this;
    return list;
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

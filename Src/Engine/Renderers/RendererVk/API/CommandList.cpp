#include "PCH.hpp"
#include "CommandList.hpp"
#include "Device.hpp"

#include "Engine/Common/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

CommandList::CommandList(CommandQueueType queueType, VkCommandBuffer commandBuffer)
    : ICommandList()
    , mQueueType(queueType)
    , mCommandBuffer(commandBuffer)
{
}

CommandList::~CommandList()
{
    // Release CB ownership back to Manager
    if (mCommandBuffer != VK_NULL_HANDLE)
        gDevice->GetCommandBufferManager(mQueueType).Free(mCommandBuffer);
}

} // namespace Renderer
} // namespace NFE

#include "PCH.hpp"
#include "CommandList.hpp"
#include "Device.hpp"


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
    if (mCommandBuffer != VK_NULL_HANDLE)
        vkFreeCommandBuffers(gDevice->GetDevice(), gDevice->GetQueueFamilyManager().GetQueueFamily(mQueueType).commandPool,
                             1, &mCommandBuffer);
}

} // namespace Renderer
} // namespace NFE

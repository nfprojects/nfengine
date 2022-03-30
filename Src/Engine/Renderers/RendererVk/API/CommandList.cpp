#include "PCH.hpp"
#include "CommandList.hpp"
#include "Device.hpp"

#include "Engine/Common/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

CommandList::CommandList(CommandQueueType queueType, VkCommandBuffer commandBuffer,
                         const UsedDescriptorSetsArray& sets)
    : ICommandList()
    , mQueueType(queueType)
    , mCommandBuffer(commandBuffer)
    , mUsedDescriptorSets(sets)
{
}

CommandList::~CommandList()
{
    // Release CB ownership back to Manager
    if (mCommandBuffer != VK_NULL_HANDLE)
    {
        UsedDescriptorSetsArray dsArray(mUsedDescriptorSets);
        gDevice->GetCommandBufferManager(mQueueType).Free(mCommandBuffer, [dsArray](){
            for (DescriptorSetCollectionID id: dsArray)
            {
                gDevice->GetDescriptorSetCache().FreeDescriptorSets(id);
            }
        });
    }
}

} // namespace Renderer
} // namespace NFE

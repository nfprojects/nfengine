#pragma once

#include "../RendererCommon/CommandRecorder.hpp"

#include "Defines.hpp"
#include "Internal/DescriptorSetCache.hpp"


namespace NFE {
namespace Renderer {

class CommandList: public ICommandList
{
    CommandQueueType mQueueType;
    VkCommandBuffer mCommandBuffer;
    UsedDescriptorSetsArray mUsedDescriptorSets;

public:
    CommandList(CommandQueueType queueType, VkCommandBuffer commandBuffer,
                const UsedDescriptorSetsArray& sets);
    ~CommandList();

    NFE_INLINE const VkCommandBuffer& GetCommandBuffer() const
    {
        return mCommandBuffer;
    }
};

} // namespace Renderer
} // namespace NFE

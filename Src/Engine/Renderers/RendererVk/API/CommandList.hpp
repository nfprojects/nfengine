#pragma once

#include "../RendererCommon/CommandRecorder.hpp"


namespace NFE {
namespace Renderer {

class CommandList: public ICommandList
{
    CommandQueueType mQueueType;
    VkCommandBuffer mCommandBuffer;

public:
    CommandList(CommandQueueType queueType, VkCommandBuffer commandBuffer);
    ~CommandList();

    NFE_INLINE const VkCommandBuffer& GetCommandBuffer() const
    {
        return mCommandBuffer;
    }
};

} // namespace Renderer
} // namespace NFE

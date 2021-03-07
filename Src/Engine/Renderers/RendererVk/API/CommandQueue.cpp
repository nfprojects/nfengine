#include "PCH.hpp"
#include "CommandQueue.hpp"


namespace NFE {
namespace Renderer {

CommandQueue::CommandQueue()
{
}

CommandQueue::~CommandQueue()
{
}

bool CommandQueue::Init(CommandQueueType type, const char* debugName)
{
    NFE_UNUSED(type);
    NFE_UNUSED(debugName);

    return false;
}

CommandQueueType CommandQueue::GetType() const
{
    return CommandQueueType::Invalid;
}

void CommandQueue::Submit(const Common::ArrayView<ICommandList*> commandLists,
                          const Common::ArrayView<IFence*> waitFences)
{
    NFE_UNUSED(commandLists);
    NFE_UNUSED(waitFences);
}

FencePtr CommandQueue::Signal(const FenceFlags flags)
{
    NFE_UNUSED(flags);

    return nullptr;
}

} // namespace Renderer
} // namespace NFE

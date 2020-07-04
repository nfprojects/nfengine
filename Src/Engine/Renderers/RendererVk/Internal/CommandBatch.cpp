#include "../PCH.hpp"
#include "CommandBatch.hpp"

#include "Commands.hpp"


namespace NFE {
namespace Renderer {


void CommandBatch::Print() const
{
    NFE_LOG_DEBUG("Commands order is :");
    for (uint32 i = 0; i < mCommands.Size(); ++i)
        NFE_LOG_DEBUG(" -> (%d) %s", mCommands[i]->GetPriority(), mCommands[i]->GetName());
}

void CommandBatch::Commit(VkCommandBuffer commandBuffer, CommandBufferState& state)
{
    // DEBUG - REMOVE
    Print();

    for (uint32 i = 0; i < mCommands.Size(); ++i)
        mCommands[i]->Execute(commandBuffer, state);
}

void CommandBatch::Clear()
{
    mCommands.Clear();
}


} // namespace Renderer
} // namespace NFE

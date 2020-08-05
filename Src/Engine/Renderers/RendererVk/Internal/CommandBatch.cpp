#include "../PCH.hpp"
#include "CommandBatch.hpp"

#include "Commands.hpp"


namespace NFE {
namespace Renderer {


const char* BatchTypeToString(CommandBatchType type)
{
    switch (type)
    {
    case CommandBatchType::Draw: return "Draw";
    case CommandBatchType::Dispatch: return "Dispatch";
    case CommandBatchType::Copy: return "Copy";
    default: return "Unknown";
    };
}


void CommandBatch::Print() const
{
    for (uint32 i = 0; i < mCommands.Size(); ++i)
        NFE_LOG_DEBUG(" -> (%d) %s", mCommands[i]->GetPriority(), mCommands[i]->GetName());
}

void CommandBatch::Submit(VkCommandBuffer commandBuffer, CommandBatchState& state)
{
    for (uint32 i = 0; i < mCommands.Size(); ++i)
        mCommands[i]->Execute(commandBuffer, state);
}

void CommandBatch::Clear()
{
    mCommands.Clear();
}


} // namespace Renderer
} // namespace NFE

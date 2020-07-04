#pragma once

#include "ICommand.hpp"
#include "API/Buffer.hpp"

#include <Engine/Common/Containers/StaticArray.hpp>



namespace NFE {
namespace Renderer {


// Keeps a batch of commands together in predefied priority order - see CommandPriority.hpp.
// It is assumed that one batch contains commands in between Draw/Dispatch/Copy calls.
// Priority queue is made to reorder calls into a predefined order:
// Compute/Copy calls:
//    - Set pipeline/bind resources
//    - Layout transitions
class CommandBatch
{
    using Batch = Common::StaticArray<ICommand*, NFE_VK_MAX_COMMANDS_IN_BATCH>;

    CommandAllocator& mCommandAllocator;
    Batch mCommands;

public:
    CommandBatch(CommandAllocator& allocator)
        : mCommandAllocator(allocator)
        , mCommands()
    {
    }

    template <typename T, typename... Args>
    void Record(Args&&... args)
    {
        ICommand* command = new (mCommandAllocator) T(std::forward<Args>(args)...);

        mCommands.PushBack(command);
        if (mCommands.Size() == 1)
            return;

        // Perform "insertion sort" - move the command back until we get sorted list
        // We perform the search from the back - it is safe to assume that commands
        // will be inserted in a close-to-proper order, which will (mostly) amortize
        // sorting process.
        int idx = static_cast<int>(mCommands.Size() - 2);
        ICommand* temp = nullptr;
        while ((idx >= 0) && (mCommands[idx]->GetPriority() > mCommands[idx + 1]->GetPriority()))
        {
            temp = mCommands[idx];
            mCommands[idx] = mCommands[idx + 1];
            mCommands[idx + 1] = temp;
            idx--;
        }
    }

    void Print() const;
    void Commit(VkCommandBuffer commandBuffer, CommandBufferState& state);
    void Clear();
};


} // namespace Renderer
} // namespace NFE

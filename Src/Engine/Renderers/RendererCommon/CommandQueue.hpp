/**
 * @file
 * @author  Witek902
 * @brief   Declarations of low-level render's ICommandQueue interface
 */

#pragma once

#include "CommandRecorder.hpp"

#include "../../Common/Containers/ArrayView.hpp"

namespace NFE {
namespace Renderer {

enum class CommandQueueType : uint8
{
    Graphics,
    Compute,
    Copy,

    Invalid,
    Max = Invalid,
};

/**
 * Class representing rendering commands queue.
 */
class ICommandQueue
{
    NFE_MAKE_NONCOPYABLE(ICommandQueue)
    NFE_MAKE_NONMOVEABLE(ICommandQueue)

public:
    ICommandQueue() = default;
    virtual ~ICommandQueue() { }

    virtual CommandQueueType GetType() const = 0;

    /**
     * Execute command lists and/or synchronize fences.
     *
     * @param commandLists  List of command lists to be executed.
     * @param waitFences    List of fences that have to complete befire the command lists can start execution.
     */
    virtual void Submit(
        const Common::ArrayView<ICommandList*> commandLists,
        const Common::ArrayView<IFence*> waitFences = Common::ArrayView<IFence*>()) = 0;

    /**
     * A helper function to execute just a single command list.
     */
    NFE_FORCE_INLINE void Execute(const CommandListPtr& commandList)
    {
        ICommandList* commandListPtr = commandList.Get();
        Submit(Common::ArrayView<ICommandList*>(&commandListPtr, 1u));
    }

    /**
     * A helper function to execute a single command list waiting for a single fence.
     */
    NFE_FORCE_INLINE void Execute(const CommandListPtr& commandList, const FencePtr& fence)
    {
        ICommandList* commandListPtr = commandList.Get();
        IFence* fencePtr = fence.Get();
        Submit(Common::ArrayView<ICommandList*>(&commandListPtr, 1u), Common::ArrayView<IFence*>(&fencePtr, 1u));
    }

    /**
     * Signal an event on the queue and return fence that can be used to sync to it.
     * Allows for waiting until all commands submitted to the queue has been completed on the GPU.
     * @return Fence object.
     */
    virtual FencePtr Signal() = 0;
};

using CommandQueuePtr = Common::SharedPtr<ICommandQueue>;


NFE_INLINE const char* CommandQueueTypeToStr(CommandQueueType type)
{
    switch (type)
    {
    case CommandQueueType::Graphics:   return "Graphics";
    case CommandQueueType::Compute:    return "Compute";
    case CommandQueueType::Copy:       return "Copy";
    case CommandQueueType::Invalid:    return "Invalid";
    default:                    NFE_FATAL("Invalid queue type");
    }

    return nullptr;
}

} // namespace Renderer
} // namespace NFE

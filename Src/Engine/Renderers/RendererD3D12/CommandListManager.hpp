/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of CommandListManager
 */

#pragma once

#include "../RendererCommon/CommandRecorder.hpp"
#include "Common.hpp"
#include "CommandList.hpp"

#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/System/RWLock.hpp"


namespace NFE {
namespace Renderer {

class CommandQueue;

class CommandListManager
{
public:
    CommandListManager();
    ~CommandListManager();

    /**
     * Request a free command list.
     * If there is no free command list in the pool, a new will be created.
     * Requested command list will be in recording state.
     */
    InternalCommandListPtr RequestCommandList(CommandQueueType queueType);

    /**
     * Called by CommandRecorder when command list is recorded (closed).
     */
    CommandListPtr OnCommandListRecorded(const InternalCommandListPtr& commandList);

    /**
     * Called by Device when a command list is queued for execution.
     */
    void ExecuteCommandList(const CommandQueue& queue, const Common::ArrayView<ICommandList*> commandLists);

    // Called by fence manager's loop when a fence value completed on GPU
    void OnFenceValueCompleted(const FenceData* fenceData, uint64 fenceValue);

private:

    Common::RWLock mLock;

    struct PerQueueTypeData
    {
        Common::DynArray<InternalCommandListPtr> commandLists;
    };

    static constexpr uint32 NumQueueTypes = (uint32)CommandQueueType::Max;
    PerQueueTypeData mPerQueueTypeData[NumQueueTypes];
};

} // namespace Renderer
} // namespace NFE

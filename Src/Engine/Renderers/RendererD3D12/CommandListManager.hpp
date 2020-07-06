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
    InternalCommandListPtr RequestCommandList();

    /**
     * Called by CommandRecorder when command list is recorded (closed).
     */
    CommandListPtr OnCommandListRecorded(const InternalCommandListPtr& commandList);

    /**
     * Called by Device when a command list is queued for execution.
     */
    bool ExecuteCommandList(const Common::ArrayView<ICommandList*> commandLists, uint64 fenceValue);

    // Called by Device when a fence value completed on GPU
    void OnFenveValueCompleted(uint64 fenceValue);

private:
    Common::RWLock mLock;
    Common::DynArray<InternalCommandListPtr> mCommandLists;
};

} // namespace Renderer
} // namespace NFE

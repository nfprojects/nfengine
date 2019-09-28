/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of CommandListManager
 */

#pragma once

#include "../RendererInterface/CommandRecorder.hpp"
#include "Common.hpp"
#include "CommandList.hpp"

#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/System/RWLock.hpp"


namespace NFE {
namespace Renderer {

class CommandListManager
{
private:
    Common::RWLock mLock;
    Common::DynArray<CommandListPtr> mCommandLists;

public:
    CommandListManager();
    ~CommandListManager();

    bool Init(ID3D12Device* device);

    /**
     * Request a free command list.
     * If there is no free command list in the pool, a new will be created.
     * Requested command list will be in recording state, that's why a command allocator is needed as parameter.
     */
    CommandList* RequestCommandList(ID3D12CommandAllocator* commandAllocator);

    /**
     * Called by CommandRecorder when command list is recorded (closed).
     */
    CommandListID OnCommandListRecorded(CommandList* commandList);

    /**
     * Called by Device when a command list is queued for execution.
     */
    bool OnExecuteCommandList(CommandListID id, uint64 frameNumber);

    /**
     * Called by Device::FinishFrame() to mark end of the previous frame.
     */
    bool OnFinishFrame();
};

} // namespace Renderer
} // namespace NFE

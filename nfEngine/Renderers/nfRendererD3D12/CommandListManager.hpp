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
    friend class Device;

    Common::RWLock mLock;
    Common::DynArray<CommandListPtr> mCommandLists;
    Common::DynArray<uint32> mMapping;

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
     * Called by Device when command list is executed.
     */
    bool OnExecuteCommandList(CommandListID id);

    /**
     * Called by Device when on FinishFrame.
     */
    bool OnFinishFrame(CommandListID id);
};

} // namespace Renderer
} // namespace NFE

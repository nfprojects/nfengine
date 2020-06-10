/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of CommandList
 */

#pragma once

#include "../Interface/CommandRecorder.hpp"
#include "Common.hpp"
#include "CommandList.hpp"
#include "Engine/Common/nfCommon.hpp"


namespace NFE {
namespace Renderer {


class CommandListManager;

/**
 * Wrapper for ID3D12CommandList.
 */
class CommandList final
{
    NFE_MAKE_NONCOPYABLE(CommandList)

    friend class CommandListManager;

public:
    enum class State
    {
        Invalid,    // not initialized
        Free,       // before CommandRecorder::Begin()
        Recording,  // after ICommandRecorder::Begin(), before ICommandRecorder::Finish()
        Recorded,   // after ICommandRecorder::Finish(), before IDevice::Execute()
    };

    explicit CommandList(CommandListID id);
    CommandList(CommandList&&) = default;
    CommandList& operator = (CommandList&&) = default;
    ~CommandList();

    /**
     * Initialize the command list.
     */
    bool Init(ID3D12CommandAllocator* commandAllocator);

    // Get raw D3D12 command list object
    ID3D12GraphicsCommandList* GetD3DCommandList() const { return mCommandList.Get(); }

    // Get command list's state
    State GetState() const { return mState; }

private:
    uint64 mFrameNumber;
    D3DPtr<ID3D12GraphicsCommandList> mCommandList;
    CommandListID mID;
    State mState;
};

using CommandListPtr = Common::UniquePtr<CommandList>;


} // namespace Renderer
} // namespace NFE

/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of CommandList
 */

#pragma once

#include "../RendererInterface/CommandRecorder.hpp"
#include "Common.hpp"
#include "CommandList.hpp"
#include "nfCommon/nfCommon.hpp"


namespace NFE {
namespace Renderer {

class CommandListManager;
class RingBuffer;

/**
 * Wrapper for ID3D12CommandList.
 */
class CommandList final
{
    NFE_MAKE_NONCOPYABLE(CommandList);

    friend class CommandListManager;

public:
    enum class State
    {
        Invalid,    // not initialized
        Free,       // before CommandRecorder::Begin()
        Recording,  // after ICommandRecorder::Begin(), before ICommandRecorder::Finish()
        Recorded,   // after ICommandRecorder::Finish(), before IDevice::Execute()
    };

    CommandList();

    /**
     * Initialize the command list.
     */
    bool Init(ID3D12CommandAllocator* commandAllocator);

    // Get raw D3D12 command list object
    ID3D12GraphicsCommandList* GetD3DCommandList() const { return mCommandList.Get(); }

    // Get command list's state
    State GetState() const { return mState; }

    CommandListID FinishRecording();
    void OnExecute();

private:
    D3DPtr<ID3D12GraphicsCommandList> mCommandList;
    CommandListID mID;
    State mState;
};

using CommandListPtr = std::unique_ptr<CommandList>;

} // namespace Renderer
} // namespace NFE

/**
 * @file
 * @author  Witek902
 * @brief   Declaration of InternalCommandList
 */

#pragma once

#include "../RendererCommon/CommandRecorder.hpp"
#include "Common.hpp"
#include "CommandList.hpp"
#include "ReferencedResourcesList.hpp"
#include "Engine/Common/Containers/WeakPtr.hpp"


namespace NFE {
namespace Renderer {


class CommandListManager;

/**
 * Wrapper for ID3D12CommandList and ID3D12CommandAllocator
 */
class InternalCommandList final
{
    NFE_MAKE_NONCOPYABLE(InternalCommandList)

    friend class CommandListManager;

public:
    enum class State
    {
        Invalid,    // not initialized
        Free,       // before CommandRecorder::Begin()
        Recording,  // after ICommandRecorder::Begin(), before ICommandRecorder::Finish()
        Recorded,   // after ICommandRecorder::Finish(), before IDevice::Execute()
        Executing,   // after IDevice::Execute()
    };

    explicit InternalCommandList(uint32 id);
    InternalCommandList(InternalCommandList&&) = default;
    InternalCommandList& operator = (InternalCommandList&&) = default;
    ~InternalCommandList();

    /**
     * Initialize the command list.
     */
    bool Init();

    // Get raw D3D12 command list object
    ID3D12GraphicsCommandList* GetD3DCommandList() const { return mCommandList.Get(); }
    ID3D12CommandAllocator* GetD3DCommandAllocator() const { return mCommandAllocator.Get(); }

    // Get command list's state
    State GetState() const { return mState; }
    ReferencedResourcesList& GetReferencedResources() { return mReferencedResources; }

private:

    void OnExecuted();

    uint64 mFrameNumber;
    D3DPtr<ID3D12GraphicsCommandList> mCommandList;
    D3DPtr<ID3D12CommandAllocator> mCommandAllocator;
    uint32 mID;
    State mState;

    // resources referenced by this commandlist
    ReferencedResourcesList mReferencedResources;
};

using InternalCommandListPtr = Common::SharedPtr<InternalCommandList>;
using InternalCommandListWeakPtr = Common::WeakPtr<InternalCommandList>;


struct CommandList : public ICommandList
{
    CommandList(InternalCommandListWeakPtr internalCommandListPtr)
        : internalCommandList(internalCommandListPtr)
    {}

    InternalCommandListWeakPtr internalCommandList;
};


} // namespace Renderer
} // namespace NFE

/**
 * @file
 * @author  Witek902
 * @brief   Declaration of InternalCommandList
 */

#pragma once

#include "../RendererCommon/CommandRecorder.hpp"
#include "ResourceStateCache.hpp"
#include "CommandList.hpp"
#include "ReferencedResourcesList.hpp"
#include "HeapAllocator.hpp"
#include "Engine/Common/Containers/WeakPtr.hpp"


namespace NFE {
namespace Renderer {

class FenceData;
class CommandListManager;

/**
 * Wrapper for ID3D12CommandList and ID3D12CommandAllocator
 */
class InternalCommandList final
{
    NFE_MAKE_NONCOPYABLE(InternalCommandList)

    friend class CommandListManager;
    friend class CommandRecorder;

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
    bool Init(CommandQueueType queueType);

    // Get raw D3D12 command list object
    ID3D12GraphicsCommandList* GetD3DCommandList() const { return mCommandList.Get(); }
    ID3D12CommandAllocator* GetD3DCommandAllocator() const { return mCommandAllocator.Get(); }

    CommandQueueType GetQueueType() const { return mQueueType; }

    // Get command list's state
    State GetState() const { return mState; }
    ReferencedResourcesList& GetReferencedResources() { return mReferencedResources; }
    ReferencedDescriptorsRanges& GetReferencedCbvSrvUavDescriptorsRanges() { return mReferencedCbvSrvUavDescriptorsRanges; }
    ReferencedDescriptorsRanges& GetReferencedSamplerDescriptorsRanges() { return mReferencedSamplerDescriptorsRanges; }

    // Generate a command list with injected resource barriers that has to be executed before the actual command list
    ID3D12GraphicsCommandList* GenerateResourceBarriersCommandList();

    void ApplyFinalResourceStates();

    void AssignFenceValue(const FenceData* fenceData, uint64 fenceValue);

private:

    void OnExecuted();

    const FenceData* mFenceData;
    uint64 mFenceValue;
    uint32 mID;
    State mState;
    CommandQueueType mQueueType;

    D3DPtr<ID3D12GraphicsCommandList> mCommandList;
    D3DPtr<ID3D12CommandAllocator> mCommandAllocator;

    // note: it's reusing the same command allocator
    D3DPtr<ID3D12GraphicsCommandList> mResourceBarriersCommandList;

    // resources referenced by this commandlist
    ReferencedResourcesList mReferencedResources;

    // temporary descriptor tables referenced by this commandlist (can be released as soon as commandlist finishes execution)
    ReferencedDescriptorsRanges mReferencedCbvSrvUavDescriptorsRanges;
    ReferencedDescriptorsRanges mReferencedSamplerDescriptorsRanges;

    ResourceStateMap mInitialResourceStates;
    ResourceStateMap mFinalResourceStates;
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

/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of CommandListManager
 */

#include "PCH.hpp"
#include "RendererD3D12.hpp"
#include "CommandListManager.hpp"
#include "CommandList.hpp"
#include "CommandQueue.hpp"
#include "Fence.hpp"
#include "Common.hpp"

#include "Engine/Common/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

using namespace Common;

InternalCommandList::InternalCommandList(uint32 id)
    : mFenceValue(FenceData::InvalidValue)
    , mFenceData(nullptr)
    , mState(State::Invalid)
    , mQueueType(CommandQueueType::Invalid)
    , mID(id)
{ }

InternalCommandList::~InternalCommandList()
{
    NFE_ASSERT(mState == State::Free || mState == State::Invalid, "Destroying command list that is being used");
}

bool InternalCommandList::Init(CommandQueueType queueType)
{
    HRESULT hr;
    ID3D12Device* device = gDevice->GetDevice();

    const D3D12_COMMAND_LIST_TYPE commandListType = TranslateCommandListType(queueType);

    hr = D3D_CALL_CHECK(device->CreateCommandAllocator(commandListType, IID_PPV_ARGS(mCommandAllocator.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create D3D12 command allocator");
        return false;
    }

    SetDebugName(mCommandAllocator.Get(), "InternalCommandList::mCommandAllocator");

    // create D3D command list (for resource barrier injection)
    hr = D3D_CALL_CHECK(device->CreateCommandList(0, commandListType, mCommandAllocator.Get(),
        nullptr, IID_PPV_ARGS(mResourceBarriersCommandList.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create D3D12 command list");
        return false;
    }

    SetDebugName(mResourceBarriersCommandList.Get(), "InternalCommandList::mResourceBarriersCommandList");

    // close immediately, we don't want that in recording state
    mResourceBarriersCommandList->Close();

    // create D3D command list
    hr = D3D_CALL_CHECK(device->CreateCommandList(0, commandListType, mCommandAllocator.Get(),
        nullptr, IID_PPV_ARGS(mCommandList.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create D3D12 command list");
        return false;
    }

    SetDebugName(mResourceBarriersCommandList.Get(), "InternalCommandList::mCommandList");

    // created D3D12 command list is in recording state by default
    mState = State::Recording;
    mQueueType = queueType;

    NFE_LOG_INFO("%s command list was created, ID = %u", CommandQueueTypeToStr(mQueueType), mID);
    return true;
}

void InternalCommandList::AssignFenceValue(const FenceData* fenceData, uint64 fenceValue)
{
    NFE_ASSERT(fenceData, "Invalid fence value");
    NFE_ASSERT(fenceValue != FenceData::InvalidValue, "Invalid fence value");
    NFE_ASSERT(fenceValue != FenceData::InitialValue, "Invalid fence value");
    NFE_ASSERT(mFenceValue == FenceData::InvalidValue, "Already has fence value");
    NFE_ASSERT(!mFenceData, "Already has fence value");

    mFenceData = fenceData;
    mFenceValue = fenceValue;
}

void InternalCommandList::OnExecuted()
{
    NFE_ASSERT(mFenceValue != FenceData::InvalidValue);
    NFE_ASSERT(mFenceValue != FenceData::InitialValue);
    NFE_ASSERT(mState == State::Executing);

    mReferencedResources.Clear();

    // release temporary descriptors
    {
        HeapAllocator& heapAllocator = gDevice->GetCbvSrvUavHeapAllocator();
        for (const HeapAllocator::DescriptorRange& range : mReferencedDescriptorsRanges)
        {
            heapAllocator.Free(range);
        }
        mReferencedDescriptorsRanges.Clear();
    }

    mState = State::Free;
    mFenceData = nullptr;
    mFenceValue = FenceData::InvalidValue;
}

ID3D12GraphicsCommandList* InternalCommandList::GenerateResourceBarriersCommandList()
{
    DynArray<D3D12_RESOURCE_BARRIER> barriers;

    D3D12_RESOURCE_BARRIER barrierDesc = {};
    barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

    for (const auto& iter : mInitialResourceStates)
    {
        const Resource* resource = iter.first;
        const ResourceState& resourceStateBefore = resource->GetState();
        const ResourceState& resourceStateAfter = iter.second;

        barrierDesc.Transition.pResource = resource->GetD3DResource();

        if (resourceStateAfter.isGlobalState && resourceStateBefore.isGlobalState)
        {
            barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrierDesc.Transition.StateBefore = resourceStateBefore.globalState;
            barrierDesc.Transition.StateAfter = resourceStateAfter.globalState;

            if (barrierDesc.Transition.StateBefore != barrierDesc.Transition.StateAfter)
            {
                barriers.PushBack(barrierDesc);
            }
        }
        else
        {
            uint32 numTransitions = Math::Max(resourceStateBefore.subresourceStates.Size(), resourceStateAfter.subresourceStates.Size());
            for (uint32 i = 0; i < numTransitions; ++i)
            {
                barrierDesc.Transition.Subresource = i;
                barrierDesc.Transition.StateBefore = resourceStateBefore.Get(i);
                barrierDesc.Transition.StateAfter = resourceStateAfter.Get(i);

                if (barrierDesc.Transition.StateBefore != barrierDesc.Transition.StateAfter)
                {
                    barriers.PushBack(barrierDesc);
                }
            }
        }
    }

    mInitialResourceStates.Clear();

    if (barriers.Empty())
    {
        // no barriers required
        return nullptr;
    }

    HRESULT hr = D3D_CALL_CHECK(mResourceBarriersCommandList->Reset(mCommandAllocator.Get(), nullptr));
    if (FAILED(hr))
    {
        return nullptr;
    }

    mResourceBarriersCommandList->ResourceBarrier(barriers.Size(), barriers.Data());

    hr = D3D_CALL_CHECK(mResourceBarriersCommandList->Close());
    if (FAILED(hr))
    {
        return nullptr;
    }

    return mResourceBarriersCommandList.Get();
}

void InternalCommandList::ApplyFinalResourceStates()
{
    for (const auto& iter : mFinalResourceStates)
    {
        Resource* resource = const_cast<Resource*>(iter.first);
        const ResourceState& finalResourceState = iter.second;

        resource->mState = finalResourceState;
    }

    mFinalResourceStates.Clear();
}

} // namespace Renderer
} // namespace NFE

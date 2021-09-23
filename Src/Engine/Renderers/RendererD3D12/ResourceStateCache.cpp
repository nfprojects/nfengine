/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of ResourceStateCache
 */

#include "PCH.hpp"
#include "ResourceStateCache.hpp"
#include "RendererD3D12.hpp"
#include "Backbuffer.hpp"

#include "Engine/Common/Logger/Logger.hpp"

namespace NFE {
namespace Renderer {


ResourceStateCache::ResourceStateCache()
{
}

ResourceStateCache::~ResourceStateCache()
{
    NFE_ASSERT(mPendingResourceBarriers.Empty(), "All barriers should be flushed at this stage");
    NFE_ASSERT(mInitialStates.Empty(), "Initial states cache should be empty at this stage");
    NFE_ASSERT(mCache.Empty(), "States cache should be empty at this stage");
}

void ResourceStateCache::OnBeginCommandBuffer()
{
    NFE_ASSERT(mPendingResourceBarriers.Empty(), "All barriers should be empty at this stage");
    NFE_ASSERT(mInitialStates.Empty(), "Initial states cache should be empty at this stage");
    NFE_ASSERT(mCache.Empty(), "States cache should be empty at this stage");
}

void ResourceStateCache::OnFinishCommandBuffer(ResourceStateMap& outExpectedInitialStates, ResourceStateMap& outFinalResourceStates)
{
    NFE_ASSERT(mPendingResourceBarriers.Empty(), "All barriers should be flushed at this stage");

    outExpectedInitialStates = std::move(mInitialStates);
    outFinalResourceStates = std::move(mCache);
}

bool ResourceStateCache::EnsureResourceState(const Resource* resource, D3D12_RESOURCE_STATES d3dState, uint32 subresource)
{
    NFE_ASSERT(resource, "NULL resource");

    if (resource->GetMode() == ResourceAccessMode::Immutable)
    {
        if (d3dState != D3D12_RESOURCE_STATE_COMMON)
        {
            const D3D12_RESOURCE_STATES allowedStates = D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_COPY_DEST;
            NFE_ASSERT((d3dState & ~allowedStates) == 0, "Illegal immutable resource state");
        }

        // immutable resources are in COMMON state and are promoted to appropriate read state automatically
        // only exception is the first resource upload
        return false;
    }

    if (resource->GetMode() == ResourceAccessMode::Upload || resource->GetMode() == ResourceAccessMode::Readback)
    {
        // upload/readback resources are in GENERIC_READ/COPY_DEST all the time
        return false;
    }

    bool pushedAnyBarrier = false;

    const auto it = mCache.Find(resource);
    if (it != mCache.End())
    {
        ResourceState& state = it->second;

        if (subresource == UINT32_MAX)
        {
            if (state.isGlobalState)
            {
                const D3D12_RESOURCE_STATES prevState = state.globalState;
                pushedAnyBarrier |= PushPendingBarrier(resource, UINT32_MAX, prevState, d3dState);
            }
            else
            {
                for (uint32 i = 0; i < state.subresourceStates.Size(); ++i)
                {
                    pushedAnyBarrier |= PushPendingBarrier(resource, i, state.subresourceStates[i], d3dState);
                }
            }
        }
        else
        {
            const D3D12_RESOURCE_STATES prevState = state.Get(subresource);
            pushedAnyBarrier |= PushPendingBarrier(resource, subresource, prevState, d3dState);
        }

        // update state in the cache
        state.Set(subresource, d3dState);
    }
    else
    {
        // TODO if seting a subresource state, it will default all other subresource states to D3D12_RESOURCE_STATE_COMMON
        // ideally, other subresource states should be undefined

        ResourceState newState;
        newState.Set(subresource, d3dState);

        mCache.Insert(resource, newState);
        mInitialStates.Insert(resource, newState);
    }

    return pushedAnyBarrier;
}

bool ResourceStateCache::PushPendingBarrier(const Resource* resource, uint32 subresource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
    NFE_ASSERT(resource, "Invalid resource");

    if (stateBefore == stateAfter)
    {
        return false;
    }

    // TODO check for duplicate barriers

    if (!mPendingResourceBarriers.Empty())
    {
        // update existing barrier
        D3D12_RESOURCE_BARRIER& lastBarrier = mPendingResourceBarriers.Back();
        if (lastBarrier.Transition.pResource == resource->GetD3DResource() &&
            lastBarrier.Transition.Subresource == subresource &&
            lastBarrier.Transition.StateAfter == stateBefore)
        {
            lastBarrier.Transition.StateAfter = stateAfter;

            if (lastBarrier.Transition.StateBefore == lastBarrier.Transition.StateAfter)
            {
                mPendingResourceBarriers.PopBack();
                return false;
            }

            return true;
        }
    }

    D3D12_RESOURCE_BARRIER barrierDesc = {};
    barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrierDesc.Transition.pResource = resource->GetD3DResource();
    barrierDesc.Transition.Subresource = subresource;
    barrierDesc.Transition.StateBefore = stateBefore;
    barrierDesc.Transition.StateAfter = stateAfter;

    mPendingResourceBarriers.PushBack(barrierDesc);

    return true;
}

void ResourceStateCache::FlushPendingBarriers(ID3D12GraphicsCommandList* d3dCommandList)
{
    if (!mPendingResourceBarriers.Empty())
    {
        d3dCommandList->ResourceBarrier(mPendingResourceBarriers.Size(), mPendingResourceBarriers.Data());

        mPendingResourceBarriers.Clear();
    }
}


} // namespace Renderer
} // namespace NFE

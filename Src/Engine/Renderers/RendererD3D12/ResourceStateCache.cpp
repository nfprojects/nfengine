/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of ResourceStateCache
 */

#include "PCH.hpp"
#include "ResourceStateCache.hpp"
#include "RendererD3D12.hpp"

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

void ResourceStateCache::EnsureResourceState(const Resource* resource, D3D12_RESOURCE_STATES d3dState, uint32 subresource)
{
    NFE_ASSERT(resource, "NULL resource");

    if (resource->GetMode() == ResourceAccessMode::Upload || resource->GetMode() == ResourceAccessMode::Readback)
    {
        // static resources are in COMMON state and are promoted to appropriate read state automatically
        // upload/readback resources are in GENERIC_READ/COPY_DEST all the time
        return;
    }

    const auto it = mCache.Find(resource);
    if (it != mCache.End())
    {
        ResourceState& state = it->second;

        if (subresource == UINT32_MAX)
        {
            if (state.isGlobalState)
            {
                const D3D12_RESOURCE_STATES prevState = state.globalState;
                PushPendingBarrier(resource, UINT32_MAX, prevState, d3dState);
            }
            else
            {
                for (uint32 i = 0; i < state.subresourceStates.Size(); ++i)
                {
                    PushPendingBarrier(resource, i, state.subresourceStates[i], d3dState);
                }
            }
        }
        else
        {
            const D3D12_RESOURCE_STATES prevState = state.Get(subresource);
            PushPendingBarrier(resource, subresource, prevState, d3dState);
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

}

void ResourceStateCache::PushPendingBarrier(const Resource* resource, uint32 subresource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
    if (stateBefore != stateAfter)
    {
        // TODO check for duplicate barriers

        D3D12_RESOURCE_BARRIER barrierDesc = {};
        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrierDesc.Transition.pResource = resource->GetD3DResource();
        barrierDesc.Transition.Subresource = subresource;
        barrierDesc.Transition.StateBefore = stateBefore;
        barrierDesc.Transition.StateAfter = stateAfter;

        mPendingResourceBarriers.PushBack(barrierDesc);
    }
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

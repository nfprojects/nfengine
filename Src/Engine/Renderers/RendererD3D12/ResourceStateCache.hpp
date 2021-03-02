/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of ResourceStateCache
 */

#pragma once

#include "Resource.hpp"

#include "Engine/Common/Containers/HashMap.hpp"


namespace NFE {
namespace Renderer {

class Resource;

struct ResourceStateCacheKey
{
    const Resource* resource;
    uint32 subResource;

    NFE_INLINE ResourceStateCacheKey(const Resource* resource, uint32 subResource)
        : resource(resource)
        , subResource(subResource)
    { }

    NFE_INLINE bool operator == (const ResourceStateCacheKey& other) const
    {
        return resource == other.resource && subResource == other.subResource;
    }
};

NFE_INLINE uint32 GetHash(const ResourceStateCacheKey& key)
{
    return Common::GetHash(key.resource) ^ Common::GetHash(key.subResource);
}


//////////////////////////////////////////////////////////////////////////

using ResourceStateMap = Common::HashMap<const Resource*, ResourceState>;

class ResourceStateCache final
{
    NFE_MAKE_NONCOPYABLE(ResourceStateCache)

public:
    ResourceStateCache();
    ~ResourceStateCache();

    // Set target expected state for given resource
    bool EnsureResourceState(const Resource* resource, D3D12_RESOURCE_STATES state, uint32 subresource = UINT32_MAX);

    // Make all requested resource state changes effective
    // This should be called just before executive command (CopyResource, Draw, Dispatch, Clear, etc.)
    void FlushPendingBarriers(ID3D12GraphicsCommandList* d3dCommandList);

    void OnBeginCommandBuffer();

    // Called on commandlist finalize
    // Returns a list what the resource states should be at the beginning of command list (required for barrier injection)
    // and a list of final resource state.
    void OnFinishCommandBuffer(ResourceStateMap& outExpectedInitialStates, ResourceStateMap& outFinalResourceStates);

private:

    bool PushPendingBarrier(const Resource* resource, uint32 subresource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);

    Common::DynArray<D3D12_RESOURCE_BARRIER> mPendingResourceBarriers;

    ResourceStateMap mCache;
    ResourceStateMap mInitialStates;
};

//////////////////////////////////////////////////////////////////////////

// A helper class that makes flushing resource barriers easier
// and tries to reduce number of unnecessary barriers.
class BarrierFlusher
{
public:
    NFE_FORCE_INLINE BarrierFlusher(ResourceStateCache& cacheObject, ID3D12GraphicsCommandList* d3dCommandList)
        : mCache(cacheObject)
        , mD3DCommandList(d3dCommandList)
    {}

    NFE_FORCE_INLINE ~BarrierFlusher()
    {
        if (mNeedFlush)
        {
            mCache.FlushPendingBarriers(mD3DCommandList);
        }
    }

    BarrierFlusher& EnsureResourceState(const Resource* resource, D3D12_RESOURCE_STATES state, uint32 subresource = UINT32_MAX)
    {
        mNeedFlush |= mCache.EnsureResourceState(resource, state, subresource);
        return *this;
    }

private:
    ResourceStateCache& mCache;
    ID3D12GraphicsCommandList* mD3DCommandList;
    bool mNeedFlush = false;
};


} // namespace Renderer
} // namespace NFE

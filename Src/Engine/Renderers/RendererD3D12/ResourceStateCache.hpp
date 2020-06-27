/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of ResourceStateCache
 */

#pragma once

#include "../RendererCommon/CommandRecorder.hpp"
#include "Common.hpp"

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


class ResourceStateCache final
{
    NFE_MAKE_NONCOPYABLE(ResourceStateCache)

public:
    ResourceStateCache();
    ~ResourceStateCache();

    /**
     * Get current resource state.
     */
    D3D12_RESOURCE_STATES GetResourceState(const Resource* resource, uint32 subResource = 0u) const;

    /**
     * Set new resource state.
     * @return  Old resource state.
     */
    D3D12_RESOURCE_STATES SetResourceState(const Resource* resource, uint32 subResource, D3D12_RESOURCE_STATES newState);

    /**
     * Called when command buffer was recorded.
     * Verifies, if all the resources are in default state.
     */
    void OnFinishCommandBuffer();

private:

    mutable Common::HashMap<ResourceStateCacheKey, D3D12_RESOURCE_STATES> mCache;
};


} // namespace Renderer
} // namespace NFE

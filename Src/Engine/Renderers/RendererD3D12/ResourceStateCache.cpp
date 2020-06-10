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
    OnFinishCommandBuffer();
}

void ResourceStateCache::OnFinishCommandBuffer()
{
#ifndef NFE_CONFIGURATION_FINAL
    // check if all resources in the cache are in default state
    for (const auto& pair : mCache)
    {
        NFE_ASSERT(pair.first.resource->GetDefaultState() == pair.second,
                   "Some of renderer resources are not in default state when finishing command buffer recording");
    }
#endif // NFE_CONFIGURATION_FINAL

    mCache.Clear();
}

D3D12_RESOURCE_STATES ResourceStateCache::GetResourceState(const Resource* resource, uint32 subResource) const
{
    NFE_ASSERT(resource, "Invalid resource");

    D3D12_RESOURCE_STATES state = resource->GetDefaultState();

    const ResourceStateCacheKey key(resource, subResource);
    const auto it = mCache.Find(key);
    if (it != mCache.End())
    {
        state = it->second;
    }
    else
    {
        // TODO double lookup could be avoided
        mCache.Insert(key, state);
    }

    return state;
}

D3D12_RESOURCE_STATES ResourceStateCache::SetResourceState(const Resource* resource, uint32 subResource, D3D12_RESOURCE_STATES newState)
{
    NFE_ASSERT(resource, "Invalid resource");

    D3D12_RESOURCE_STATES oldState = resource->GetDefaultState();

    const ResourceStateCacheKey key(resource, subResource);
    const auto it = mCache.Find(key);
    if (it != mCache.End())
    {
        // TODO double lookup could be avoided
        D3D12_RESOURCE_STATES& entry = mCache[key];
        oldState = entry;
        entry = newState;
    }
    else
    {
        // TODO double lookup could be avoided
        mCache.Insert(key, newState);
    }

    return oldState;
}


} // namespace Renderer
} // namespace NFE
/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of ResourceStateCache
 */

#include "PCH.hpp"
#include "ResourceStateCache.hpp"
#include "RendererD3D12.hpp"

#include "nfCommon/Logger/Logger.hpp"

namespace NFE {
namespace Renderer {


ResourceStateCache::ResourceStateCache()
{
}

ResourceStateCache::~ResourceStateCache()
{
    // TODO checkif all resources in the cache are in default state
}

D3D12_RESOURCE_STATES ResourceStateCache::GetResourceState(const IResource* resource, uint32 subResource) const
{
    const CacheKey key(resource, subResource);
}

D3D12_RESOURCE_STATES ResourceStateCache::SetResourceState(const IResource* resource, uint32 subResource, D3D12_RESOURCE_STATES newState)
{
    const CacheKey key(resource, subResource);
}


} // namespace Renderer
} // namespace NFE

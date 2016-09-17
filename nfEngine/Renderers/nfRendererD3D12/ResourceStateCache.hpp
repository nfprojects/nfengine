/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of ResourceStateCache
 */

#pragma once

#include "../RendererInterface/CommandRecorder.hpp"
#include "Common.hpp"

#include "nfCommon/nfCommon.hpp"
#include "nfCommon/Containers/HashMap.hpp"

namespace NFE {
namespace Renderer {

class IResource;

class ResourceStateCache final
{
    NFE_MAKE_NONCOPYABLE(ResourceStateCache);

public:
    ResourceStateCache();
    ~ResourceStateCache();

    /**
     * Get current resource state.
     */
    D3D12_RESOURCE_STATES GetResourceState(const IResource* resource, uint32 subResource = 0u) const;

    /**
     * Set new resource state.
     * @return  Old resource state.
     */
    D3D12_RESOURCE_STATES SetResourceState(const IResource* resource, uint32 subResource, D3D12_RESOURCE_STATES newState);

private:
    struct CacheKey
    {
        const IResource* resource;
        uint32 subResource;

        NFE_INLINE CacheKey(const IResource* resource, uint32 subResource)
            : resource(resource)
            , subResource(subResource)
        { }
    };

    Common::HashMap<CacheKey, D3D12_RESOURCE_STATES> mCache;
};


} // namespace Renderer
} // namespace NFE

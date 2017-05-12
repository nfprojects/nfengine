/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource manager declarations.
 */

#pragma once

#include "../Core.hpp"
#include "Resource.hpp"


namespace NFE {
namespace Resource {

using LoadingTokenPtr = std::shared_ptr<LoadingToken>;


/**
 * Resources manager - class responsible for issuing resource loading
 * and keeping track of all loaded resources.
 */
class CORE_API ResourceManager final
{
    NFE_MAKE_NONCOPYABLE(ResourceManager)
    NFE_MAKE_NONMOVEABLE(ResourceManager)

public:
    struct Stats
    {
        // number of loaded resources
        uint32 loadedResources;

        // number of resources being loaded in this moment
        uint32 loadingResources;

        // number of resources enqueued for loading
        uint32 queuedResources;

        // TODO: memory usage, etc.

        Stats()
            : loadedResources(0)
            , loadingResources(0)
            , queuedResources(0)
        {}
    };

private:
    using Lock = std::unique_lock<std::mutex>;

    std::unique_ptr<Common::AsyncThreadPool> mThreadPool;

    // list of loaded resources
    std::mutex mLoadedResourcesMutex;
    std::unordered_map<ResourceName, ResourceWeakPtr> mLoadedResources;

    // TODO
    // Resources which ref counter reaches 1 (only res manager reference) should be moved
    // to "unreferenced" resources list and stay there for some period.
    // This should be connected to memory management - i.e. if we are tight on the memory
    // budged, the resources from this list should be flushed.


    // list of tokens of resources being loaded / queued
    std::mutex mLoadingResourcesMutex;
    std::unordered_map<ResourceName, LoadingTokenPtr> mLoadingResources;

public:
    ResourceManager();
    ~ResourceManager();

    /**
     * Issue resource loading and returns loading token.
     */
    LoadingTokenPtr StartResourceLoading(const ResourceName& name);

    /**
     * Issue custom resource creation.
     * @note    The name must not conflict with any regular (non-custom) resource.
     * @param   userData    Pointer passed to the resource initialization method.
     * @return  Loading token (custom resource creation shares the same thread pool).
     */
    LoadingTokenPtr CreateCustomResource(const ResourceName& name, void* userData);

    /**
     * Check if a given resource is loaded.
     */
    bool IsResourceLoaded(const ResourceName& name);

    /**
     * Get already loaded resource.
     */
    ResourcePtr GetLoadedResource(const ResourceName& name);

private:
    LoadingTokenPtr GetExistingToken_NoLock(const ResourceName& name);
    ResourcePtr GetLoadedResource_NoLock(const ResourceName& name);
};

} // namespace Resource
} // namespace NFE

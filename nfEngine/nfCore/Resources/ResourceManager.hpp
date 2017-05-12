/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource manager declarations.
 */

#pragma once

#include "../Core.hpp"
#include "Resource.hpp"

#include "nfCommon/Containers/SharedPtr.hpp"


namespace NFE {
namespace Resource {

using LoadingTokenPtr = Common::SharedPtr<LoadingToken>;


/**
 * Resources manager.
 * Class responsible for issuing resource loading and keeping track of all loaded/loading resources.
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

    ResourceManager();
    ~ResourceManager();

    /**
     * Enqueue resource loading and returns loading token.
     */
    LoadingTokenPtr EnqueueResourceLoading(const ResourceName& name);

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
     * @note    Will return nullptr if the resource is not loaded.
     * @note    This won't start resource loading. Use EnqueueResourceLoading for that.
     */
    ResourcePtr GetLoadedResource(const ResourceName& name);

    /**
     * Get resource manager statistics.
     */
    void GetStats(Stats& outStats);

private:
    LoadingTokenPtr GetExistingToken_NoLock(const ResourceName& name);
    ResourcePtr GetLoadedResource_NoLock(const ResourceName& name);

    // thread pool used for resource loading
    // TODO use one global thread pool (Common::ThreadPool) after refactor
    Common::UniquePtr<Common::AsyncThreadPool> mThreadPool;

    // list of loaded resources
    Common::RWLock mLoadedResourcesLock;
    std::unordered_map<ResourceName, ResourceWeakPtr> mLoadedResources; // TODO use Common::HashMap

    // list of tokens of resources being loaded / queued
    Common::RWLock mLoadingResourcesLock;
    std::unordered_map<ResourceName, LoadingTokenPtr> mLoadingResources; // TODO use Common::HashMap


    // TODO
    // Resources which ref counter reaches 1 (only res manager reference) should be moved
    // to "unreferenced" resources list and stay there for some period.
    // This should be connected to memory management - i.e. if we are tight on the memory
    // budged, the resources from this list should be flushed.
};

} // namespace Resource
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../Core.hpp"
#include "ResourceName.hpp"

#include "nfCommon/Containers/SharedPtr.hpp"
#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/System/RWLock.hpp"

#include <functional>


namespace NFE {
namespace Resource {

using ResourcePtr = Common::SharedPtr<ResourceBase>;

/**
 * Helper class allowing for state tracking of resource being loaded.
 * Until the token exists, the resource is queued to be loaded.
 */
class CORE_API LoadingToken final
{
    NFE_MAKE_NONCOPYABLE(LoadingToken);
    NFE_MAKE_NONMOVEABLE(LoadingToken);

public:
    enum class Status
    {
        Enqueued,   // resource is enqueued to be loaded (waiting in threadpool queue)
        Loading,    // resource is being loaded right now
        Loaded,     // resource is loaded (GetResource will return non-nullptr)
        Failed,     // resource loading failed
    };

    using PostLoadFunc = std::function<void(const ResourcePtr&)>;

    LoadingToken(const ResourceName& name, Common::TaskID loadingTask);
    LoadingToken(const ResourceName& name, const ResourcePtr& resource);
    ~LoadingToken();

    /**
     * Wait until the resource is loaded.
     * @note    This method can be called only from the main thread.
     * @return  Resource handle or nullptr if loading failed.
     */
    ResourcePtr WaitForResource();

    /**
     * Get resource (returns nullptr if not loaded).
     */
    ResourcePtr GetResource() const { return mLoadedResource; }

    /**
     * Get loading status.
     */
    Status GetStatus() const;

    /**
     * Get internal loading task ID.
     */
    Common::TaskID GetLoadingTask() { return mLoadingTask; }

    /**
     * Add a function that will be loaded after the resource is loaded.
     * @note    If the resource is already loaded, the function will execute immediately.
     */
    bool AddPostLoadFunc(const PostLoadFunc& func);

private:
    ResourceName mName;
    ResourcePtr mLoadedResource;
    Common::TaskID mLoadingTask;

    Common::RWLock mPostLoadFunctionsLock;
    Common::DynArray<PostLoadFunc> mPostLoadFunctions;
};

} // namespace Resource
} // namespace NFE

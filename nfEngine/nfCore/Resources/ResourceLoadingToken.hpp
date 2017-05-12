/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../Core.hpp"


namespace NFE {
namespace Resource {

using ResourcePtr = std::shared_ptr<ResourceBase>;

/**
 * Helper class describing state of loaded resource or a resource being loaded.
 * Until the token exists, the resource is queued to be loaded.
 */
class CORE_API LoadingToken final
{
public:
    explicit LoadingToken(Common::TaskID loadingTask);
    explicit LoadingToken(const ResourcePtr& resource);
    ~LoadingToken();

    /**
     * Wait until the resource is loaded.
     * @note    This method can be called only from the main thread.
     * @return  Resource handle or nullptr if loading failed.
     */
    ResourcePtr WaitForResource();

    /**
     * Get loaded resource.
     */
    ResourcePtr GetResource() const { return mLoadedResource; }

    /**
     * Is the resource loaded?
     */
    bool IsLoaded() const { return mLoadedResource != nullptr; }

    Common::TaskID GetLoadingTask() { return mLoadingTask; }

private:
    ResourcePtr mLoadedResource;
    Common::TaskID mLoadingTask;
};

} // namespace Resource
} // namespace NFE

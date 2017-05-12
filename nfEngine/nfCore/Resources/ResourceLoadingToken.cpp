/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "ResourceLoadingToken.hpp"

#include "nfCommon/System/Memory.hpp"
#include "nfCommon/System/Timer.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/Utils/AsyncThreadPool.hpp"


namespace NFE {
namespace Resource {

LoadingToken::LoadingToken(const ResourceName& name, Common::TaskID loadingTask)
    : mName(name)
    , mLoadingTask(loadingTask)
{
}

LoadingToken::LoadingToken(const ResourceName& name, const ResourcePtr& resource)
    : mName(name)
    , mLoadedResource(resource)
{
    // Note: 'loadedResource' can be null here. That means that it failed to load before.
}

LoadingToken::~LoadingToken()
{
    // TODO cancel resource loading
}

ResourcePtr LoadingToken::WaitForResource()
{
    if (mLoadedResource)
    {
        // resource is already loaded
        return mLoadedResource;
    }

    Common::Timer timer;
    timer.Start();
    // TODO wait on mLoadingTask
    const float waitTime = static_cast<float>(timer.Stop());

    LOG_WARNING("Waiting on resource '%s' took %f ms", mName.Str(), waitTime);

    return mLoadedResource;
}

bool LoadingToken::AddPostLoadFunc(const PostLoadFunc& func)
{
    if (mLoadedResource)
    {
        // resource is already loaded
        func(mLoadedResource);
        return true;
    }

    Common::ScopedExclusiveLock lock(mPostLoadFunctionsLock);
    return mPostLoadFunctions.PushBack(func) != mPostLoadFunctions.End();
}

} // namespace Resource
} // namespace NFE

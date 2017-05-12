/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource manager definition.
 */

#include "PCH.hpp"
#include "ResourceManager.hpp"
#include "nfCommon/System/Memory.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/Utils/ThreadPool.hpp"
#include "nfCommon/Utils/AsyncThreadPool.hpp"
#include "nfCommon/Utils/ScopedLock.hpp"
#include "nfCommon/System/Assertion.hpp"


namespace NFE {
namespace Resource {


class WaitableTask
{
private:
    Common::TaskID mTaskID;

public:
    void Wait(Common::ThreadPool& pool)
    {
        if (mTaskID != NFE_INVALID_TASK_ID)
        {
            pool.WaitForTask(mTaskID);
        }
    }
};


ResourceManager::ResourceManager()
{
    mThreadPool.reset(new Common::AsyncThreadPool);
}

ResourceManager::~ResourceManager()
{
    // TODO cleanup
}

LoadingTokenPtr ResourceManager::StartResourceLoading(const ResourceName& name)
{
    Lock lock(mLoadedResourcesMutex);

    LoadingTokenPtr existingToken = GetExistingToken_NoLock(name);
    if (existingToken)
    {
        // the token already existed - don't create a new one
        return existingToken;
    }

    // TODO spawn tasks:
    // 1. load resource from disk (async)
    // 2. OnLoad() [spawns sub-resources loading]
    // 3. OnPostLoad() [resource is fully loaded, with all dependencies]
}

LoadingTokenPtr ResourceManager::CreateCustomResource(const ResourceName& name, void* userData)
{
    Lock lock(mLoadedResourcesMutex);
}

bool ResourceManager::IsResourceLoaded(const ResourceName& name)
{
    Lock lock(mLoadedResourcesMutex);
    return mLoadedResources.count(name) > 0;
}

ResourcePtr ResourceManager::GetLoadedResource(const ResourceName& name)
{
    Lock lock(mLoadedResourcesMutex);
    return GetLoadedResource_NoLock(name);
}

LoadingTokenPtr ResourceManager::GetExistingToken_NoLock(const ResourceName& name)
{
    const auto iter = mLoadingResources.find(name);
    if (iter == mLoadingResources.end())
    {
        // resource has no token
        return LoadingTokenPtr();
    }

    LoadingTokenPtr token = iter->second;
    NFE_ASSERT(token, "Expected valid token. This indicates resource manager bug");
    return token;
}

ResourcePtr ResourceManager::GetLoadedResource_NoLock(const ResourceName& name)
{
    const auto iter = mLoadedResources.find(name);
    if (iter == mLoadedResources.end())
    {
        // resource not loaded
        return ResourcePtr();
    }

    ResourcePtr resource = iter->second.lock();
    NFE_ASSERT(resource, "Expected valid resource pointer. This indicates resource manager bug");
    return resource;
}

/*

ResourceManager::ResourceManager()
{
    mThreadPool.reset(new Common::AsyncThreadPool);
}

ResourceManager::~ResourceManager()
{
}


void ResourceManager::Release()
{
    // unload all resources
    NFE_LOG_INFO("Unloading resources...");
    std::map<const char*, ResourceBase*, CompareResName>::iterator it;
    for (it = mResources.begin(); it != mResources.end(); it++)
    {
        UnloadResource((*it).second);
    }

    mThreadPool->WaitForAllTasks();
    mThreadPool.reset();

    // delete all resources
    NFE_LOG_INFO("Deleting resources...");
    for (it = mResources.begin(); it != mResources.end(); it++)
    {
        ResourceBase* resource = (*it).second;
        delete resource;
    }
    mResources.clear();
}

ResourceBase* ResourceManager::GetResource(const char* name, ResourceType type, bool check)
{
    if (name == nullptr)
    {
        NFE_LOG_ERROR("Resource name is NULL");
        return nullptr;
    }

    Common::ScopedMutexLock ulock(mResListMutex);

    ResourceBase* resource = nullptr;

    // check resources list
    if (mResources.count(name))
    {
        resource = mResources[name];
        return resource;
    }

    // resource does not exist
    if (check)
        return nullptr;

    // TODO use RTTI system for this...
    switch (type)
    {
        case ResourceType::Shader:
            resource = new Multishader;
            break;

        case ResourceType::Texture:
            resource = new Texture;
            break;

        case ResourceType::Material:
            resource = new Material;
            break;

        case ResourceType::Mesh:
            resource = new Mesh;
            break;

        case ResourceType::CollisionShape:
            resource = new CollisionShape;
            break;

        default:
            ulock.Unlock();
            NFE_LOG_ERROR("Wrong 'type' argument");
            return nullptr;
    }

    if (resource == nullptr)
    {
        ulock.Unlock();
        NFE_LOG_ERROR("Memory allocation failed");
        return nullptr;
    }

    // copy resource name
    strcpy(resource->mName, name);

    // add resource to the resources map
    std::pair<const char*, ResourceBase*> resPair;
    resPair.first = resource->mName;
    resPair.second = resource;
    mResources.insert(resPair);

    ulock.Unlock();

    if (name[0] == g_CustomResourcePrefix)
    {
        resource->mCustom = true;
    }

    return resource;
}

bool ResourceManager::DeleteResource(const char* name)
{
    Common::ScopedMutexLock ulock(mResListMutex);
    std::map<const char*, ResourceBase*, CompareResName>::iterator it;

    it = mResources.find(name);
    if (it != mResources.end())
    {
        ResourceBase* resource = it->second;

        // check if resource has no references
        if (resource->GetState() == ResourceState::Unloaded)
        {
            mResources.erase(it);
            ulock.Unlock();
            delete resource;
            return true;
        }

        ulock.Unlock();
        NFE_LOG_WARNING("Can't remove resource '%s', because it's still loaded", name);
        return false;
    }
    return true;
}

bool ResourceManager::AddCustomResource(ResourceBase* resource, const char* name)
{
    if (!Common::MemoryCheck(resource))
    {
        NFE_LOG_ERROR("Memory pointed by resource pointer is corrupted.");
        return false;
    }

    Common::ScopedMutexLock ulock(mResListMutex);

    // check if resource already exists
    const char* pNameToCheck = (name != nullptr) ? name : resource->mName;
    if (mResources.count(pNameToCheck) > 0)
    {
        ulock.Unlock();
        NFE_LOG_ERROR("Resource with name '%s' already exists.", pNameToCheck);
        return false;
    }

    if (name != nullptr)
    {
        // copy resource name
        strcpy(resource->mName, name);
    }

    // add resource to resources map
    std::pair<const char*, ResourceBase*> resPair;
    resPair.first = resource->mName;
    resPair.second = resource;
    mResources.insert(resPair);

    return true;
}

void ResourceManager::LoadResource(ResourceBase* resource)
{
    resource->mDestState.store(ResourceState::Loaded);
    resource->mFuncID = mThreadPool->Enqueue(std::bind(&ResourceBase::Load, resource));
}

void ResourceManager::UnloadResource(ResourceBase* resource)
{
    resource->mDestState.store(ResourceState::Unloaded);
    resource->mFuncID = mThreadPool->Enqueue(std::bind(&ResourceBase::Unload, resource));
}

void ResourceManager::ReloadResource(ResourceBase* resource)
{
    auto resourceReloadFunc = [resource] ()
    {
        ResourceState expected = ResourceState::Loaded;
        resource->mState.compare_exchange_strong(expected, ResourceState::Unloading);
        if (expected == ResourceState::Loaded)
        {
            resource->OnUnload();
            resource->SetState(ResourceState::Loading);
            bool result = resource->OnLoad();
            resource->SetState(result ? ResourceState::Loaded : ResourceState::Failed);
        }
    };

    if (resource->GetState() == ResourceState::Loaded)
        resource->mFuncID = mThreadPool->Enqueue(std::bind(resourceReloadFunc));
}

bool ResourceManager::WaitForResource(ResourceBase* resource)
{
    if (!resource)
    {
        NFE_LOG_ERROR("Invalid resource pointer");
        return false;
    }

    mThreadPool->WaitForTask(resource->mFuncID);
    return true;
}

*/

} // namespace Resource
} // namespace NFE

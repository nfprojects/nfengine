/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource manager definition.
 */

#include "PCH.hpp"
#include "ResourcesManager.hpp"
#include "../nfCommon/Memory.hpp"
#include "../nfCommon/Logger.hpp"

// TODO: remove these dependencies - adding a new resource type shouldn't force programmer to modify this file...
#include "Texture.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "CollisionShape.hpp"
#include "SoundSample.hpp"

namespace NFE {
namespace Resource {

// TEMPORARY
const char g_CustomResourcePrefix = '/';


ResManager::ResManager()
{
    mThreadPool.reset(new Common::ThreadPool);
}

ResManager::~ResManager()
{
}

void ResManager::Release()
{
    // unload all resources
    LOG_INFO("Unloading resources...");
    std::map<const char*, ResourceBase*, CompareResName>::iterator it;
    for (it = mResources.begin(); it != mResources.end(); it++)
    {
        UnloadResource((*it).second);
    }

    mThreadPool->WaitForAllTasks();
    mThreadPool.reset();

    // delete all resources
    LOG_INFO("Deleting resources...");
    for (it = mResources.begin(); it != mResources.end(); it++)
    {
        ResourceBase* resource = (*it).second;
        delete resource;
    }
    mResources.clear();
}

ResourceBase* ResManager::GetResource(const char* pName, ResourceType type, bool check)
{
    std::unique_lock<std::mutex> ulock(mResListMutex);

    ResourceBase* resource = nullptr;

    // check resources list
    if (mResources.count(pName))
    {
        resource = mResources[pName];
        return resource;
    }

    // resource does not exist
    if (check)
        return nullptr;

    switch (type)
    {
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

        case ResourceType::Sound:
            resource = new SoundSample;
            break;

        default:
            ulock.unlock();
            LOG_ERROR("Wrong 'type' argument");
            return nullptr;
    }

    if (resource == nullptr)
    {
        ulock.unlock();
        LOG_ERROR("Memory allocation failed");
        return nullptr;
    }

    // copy resource name
    strcpy(resource->mName, pName);

    // add resource to the resources map
    std::pair<const char*, ResourceBase*> resPair;
    resPair.first = resource->mName;
    resPair.second = resource;
    mResources.insert(resPair);

    ulock.unlock();

    if (pName[0] == g_CustomResourcePrefix)
    {
        resource->mCustom = true;
    }

    return resource;
}

bool ResManager::DeleteResource(const char* pName)
{
    std::unique_lock<std::mutex> ulock(mResListMutex);
    std::map<const char*, ResourceBase*, CompareResName>::iterator it;

    it = mResources.find(pName);
    if (it != mResources.end())
    {
        ResourceBase* resource = it->second;

        // check if resource has no references
        if (resource->GetState() == ResourceState::Unloaded)
        {
            mResources.erase(it);
            ulock.unlock();
            delete resource;
            return true;
        }

        ulock.unlock();
        LOG_WARNING("Can't remove resource '%s', because it's still loaded", pName);
        return false;
    }
    return true;
}

Result ResManager::AddCustomResource(ResourceBase* pResource, const char* pName)
{
    using namespace Util;

    if (!(Common::MemoryCheck(pResource) & (ACCESS_READ | ACCESS_WRITE)))
    {
        LOG_ERROR("Memory pointed by pResource pointer is corrupted.");
        return Result::CorruptedPointer;
    }

    std::unique_lock<std::mutex> ulock(mResListMutex);

    // check if resource already exists
    const char* pNameToCheck = (pName != nullptr) ? pName : pResource->mName;
    if (mResources.count(pNameToCheck) > 0)
    {
        ulock.unlock();
        LOG_ERROR("Resource with name '%s' already exists.", pNameToCheck);
        return Result::Error;
    }

    if (pName != nullptr)
    {
        // copy resource name
        strcpy(pResource->mName, pName);
    }

    // add resource to resources map
    std::pair<const char*, ResourceBase*> resPair;
    resPair.first = pResource->mName;
    resPair.second = pResource;
    mResources.insert(resPair);

    return Result::OK;
}

bool ResManager::IsBusy()
{
    // TODO
    return false;
}

void ResManager::LoadResource(ResourceBase* pResource)
{
    pResource->mDestState.store(ResourceState::Loaded);
    mThreadPool->Enqueue(std::bind(&ResourceBase::Load, pResource));
}

void ResManager::UnloadResource(ResourceBase* pResource)
{
    pResource->mDestState.store(ResourceState::Unloaded);
    mThreadPool->Enqueue(std::bind(&ResourceBase::Unload, pResource));
}

void ResManager::ReloadResource(ResourceBase* pResource)
{
    auto resourceReloadFunc = [pResource] ()
    {
        ResourceState expected = ResourceState::Loaded;
        pResource->mState.compare_exchange_strong(expected, ResourceState::Unloading);
        if (expected == ResourceState::Loaded)
        {
            pResource->OnUnload();
            pResource->SetState(ResourceState::Loading);
            bool result = pResource->OnLoad();
            pResource->SetState(result ? ResourceState::Loaded : ResourceState::Failed);
        }
    };

    if (pResource->GetState() == ResourceState::Loaded)
        mThreadPool->Enqueue(std::bind(resourceReloadFunc));
}

} // namespace Resource
} // namespace NFE

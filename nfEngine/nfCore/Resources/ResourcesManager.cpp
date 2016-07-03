/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource manager definition.
 */

#include "PCH.hpp"
#include "ResourcesManager.hpp"
#include "nfCommon/Memory.hpp"
#include "nfCommon/Logger.hpp"

// TODO: remove these dependencies - adding a new resource type shouldn't force programmer to modify this file...
#include "Multishader.hpp"
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
    mThreadPool.reset(new Common::AsyncThreadPool);
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

ResourceBase* ResManager::GetResource(const char* name, ResourceType type, bool check)
{
    if (name == nullptr)
    {
        LOG_ERROR("Resource name is NULL");
        return nullptr;
    }

    std::unique_lock<std::mutex> ulock(mResListMutex);

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
    strcpy(resource->mName, name);

    // add resource to the resources map
    std::pair<const char*, ResourceBase*> resPair;
    resPair.first = resource->mName;
    resPair.second = resource;
    mResources.insert(resPair);

    ulock.unlock();

    if (name[0] == g_CustomResourcePrefix)
    {
        resource->mCustom = true;
    }

    return resource;
}

bool ResManager::DeleteResource(const char* name)
{
    std::unique_lock<std::mutex> ulock(mResListMutex);
    std::map<const char*, ResourceBase*, CompareResName>::iterator it;

    it = mResources.find(name);
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
        LOG_WARNING("Can't remove resource '%s', because it's still loaded", name);
        return false;
    }
    return true;
}

Result ResManager::AddCustomResource(ResourceBase* resource, const char* name)
{
    using namespace Util;

    if (!Common::MemoryCheck(resource))
    {
        LOG_ERROR("Memory pointed by resource pointer is corrupted.");
        return Result::CorruptedPointer;
    }

    std::unique_lock<std::mutex> ulock(mResListMutex);

    // check if resource already exists
    const char* pNameToCheck = (name != nullptr) ? name : resource->mName;
    if (mResources.count(pNameToCheck) > 0)
    {
        ulock.unlock();
        LOG_ERROR("Resource with name '%s' already exists.", pNameToCheck);
        return Result::Error;
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

    return Result::OK;
}

void ResManager::LoadResource(ResourceBase* resource)
{
    resource->mDestState.store(ResourceState::Loaded);
    resource->mFuncID = mThreadPool->Enqueue(std::bind(&ResourceBase::Load, resource));
}

void ResManager::UnloadResource(ResourceBase* resource)
{
    resource->mDestState.store(ResourceState::Unloaded);
    resource->mFuncID = mThreadPool->Enqueue(std::bind(&ResourceBase::Unload, resource));
}

void ResManager::ReloadResource(ResourceBase* resource)
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

bool ResManager::WaitForResource(ResourceBase* resource)
{
    if (!resource)
    {
        LOG_ERROR("Invalid resource pointer");
        return false;
    }

    mThreadPool->WaitForTask(resource->mFuncID);
    return true;
}

} // namespace Resource
} // namespace NFE

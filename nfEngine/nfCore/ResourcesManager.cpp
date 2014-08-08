/**
    NFEngine project

    \file   ResourcesManager.cpp
    \brief  Resource manager definition.
*/

#include "stdafx.h"
#include "ResourcesManager.h"
#include "Memory.h"
#include "../nfCommon/Logger.h"

// TODO: remove these dependencies - adding a new resource type shouldn't force programmer to modify this file...
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "CollisionShape.h"
#include "SoundSample.h"

namespace NFE {
namespace Resource {

// TEMPORARY
const char g_CustomResourcePrefix = '/';


ResManager::ResManager()
{
    mThreadPool.Init();
}

ResManager::~ResManager()
{
    //unload all resources
    LOG_INFO("Unloading resources...");
    std::map<const char*, ResourceBase*, CompareResName>::iterator it;
    for (it = mResources.begin(); it != mResources.end(); it++)
    {
        UnloadResource((*it).second);
    }
    mThreadPool.WaitForAllTasks();
    mThreadPool.Release();

    //delete all resources
    LOG_INFO("Deleting resources...");
    for (it = mResources.begin(); it != mResources.end(); it++)
    {
        ResourceBase* pResource = (*it).second;
        delete pResource;
    }
    mResources.clear();

    //Packer_ReleaseReader();
}


//This function returns pointer to a resource by name and type
//If the resource does not exist, the function starts loading
ResourceBase* ResManager::GetResource(const char* pName, ResourceType Type, bool check)
{
    //STL is not thread-safe - manual synchronization is needed
    std::unique_lock<std::mutex> ulock(mResListMutex);

    ResourceBase* pResource = NULL;

    // check resources list
    if (mResources.count(pName))
    {
        pResource = mResources[pName];
        //LoadResource(pResource);
        return pResource;
    }

    //resource does not exist
    if (check)
        return 0;

    switch (Type)
    {
        case ResourceType::TEXTURE:
            pResource = new Texture;
            break;

        case ResourceType::MATERIAL:
            pResource = new Material;
            break;

        case ResourceType::MESH:
            pResource = new Mesh;
            break;

        case ResourceType::COLLISION_SHAPE:
            pResource = new CollisionShape;
            break;

        case ResourceType::SOUND:
            pResource = new SoundSample;
            break;

        default:
            ulock.unlock();
            LOG_ERROR("Wrong 'type' argument");
            return 0;
    }

    if (pResource == 0)
    {
        ulock.unlock();
        LOG_ERROR("Memory allocation failed");
        return 0;
    }

    //copy resource name
    strcpy(pResource->mName, pName);

    //add resource to the resources map
    std::pair<const char*, ResourceBase*> resPair;
    resPair.first = pResource->mName;
    resPair.second = pResource;
    mResources.insert(resPair);

    ulock.unlock();

    if (pName[0] == g_CustomResourcePrefix)
    {
        pResource->mCustom = true;
    }

    return pResource;
}

//Delete resource from map (if the resource has no references)
int ResManager::DeleteResource(const char* pName)
{
    std::unique_lock<std::mutex> ulock(mResListMutex); //STL is not thread-safe!
    std::map<const char*, ResourceBase*, CompareResName>::iterator it;
    ResourceBase* pResource = 0;

    it = mResources.find(pName);
    if (it != mResources.end())
    {
        pResource = it->second;

        //check if resource has no references
        if (pResource->GetState() == RES_UNLOADED)
        {
            mResources.erase(it);
            ulock.unlock();
            delete pResource;
            return 0;
        }
        return 1;
    }
    return 0;
}

// Insert resource object created by the user.
Result ResManager::AddCustomResource(ResourceBase* pResource, const char* pName)
{
    using namespace Util;

    if (!(MemoryCheck(pResource) & (ACCESS_READ | ACCESS_WRITE)))
    {
        LOG_ERROR("Memory pointed by pResource pointer is corrupted.");
        return Result::CorruptedPointer;
    }

    //STL is not thread-safe!
    std::unique_lock<std::mutex> ulock(mResListMutex);

    //check if resource already exists
    const char* pNameToCheck = (pName != NULL) ? pName : pResource->mName;
    if (mResources.count(pNameToCheck) > 0)
    {
        ulock.unlock();
        LOG_ERROR("Resource with name '%s' already exists.", pNameToCheck);
        return Result::Error;
    }

    if (pName != NULL)
    {
        //copy resource name
        strcpy(pResource->mName, pName);
    }

    //add resource to resources map
    std::pair<const char*, ResourceBase*> resPair;
    resPair.first = pResource->mName;
    resPair.second = pResource;
    mResources.insert(resPair);

    return Result::OK;
}

bool ResManager::IsBusy()
{
    return mThreadPool.GetLoad() > 0;
}


//called by thread pool
void ResourceLoadingCallback(void* pParam, int Instance, int ThreadID)
{
    ResourceBase* pResource = (ResourceBase*)pParam;
    pResource->Load();

    /*
    if (InterlockedCompareExchange(&pResource->m_State, RES_LOADING, RES_UNLOADED) == RES_UNLOADED)
    {
        //destination status changed during task waiting
        if (pResource->m_DestState == RES_UNLOADED)
        {
            pResource->SetState(RES_UNLOADED);
            return;
        }

        for(;;)
        {
            bool result = pResource->OnLoad();

            if (pResource->m_DestState == RES_LOADED)
            {
                pResource->SetState(result ? RES_LOADED : RES_FAILED);
                return;
            }

            //destination status changed during resource loading - unload resource
            pResource->SetState(RES_UNLOADING);
            pResource->OnUnload();

            if (pResource->m_DestState == RES_UNLOADED)
            {
                pResource->SetState(RES_UNLOADED);
                return;
            }
        }
    }
    */
}

//called by thread pool
void ResourceUnloadingCallback(void* pParam, int Instance, int ThreadID)
{
    ResourceBase* pResource = (ResourceBase*)pParam;
    pResource->Unload();

    /*
    if (InterlockedCompareExchange(&pResource->m_State, RES_UNLOADING, RES_LOADED) == RES_LOADED)
    {
        //destination status changed during task waiting
        if (pResource->m_DestState == RES_LOADED)
        {
            pResource->SetState(RES_LOADED);
            return;
        }

        for(;;)
        {
            pResource->OnUnload();
            if (pResource->m_DestState == RES_UNLOADED)
            {
                pResource->SetState(RES_UNLOADED);
                return;
            }

            //destination status changed during resource loading - load resource back
            pResource->SetState(RES_LOADING);
            bool result = pResource->OnLoad();

            if (pResource->m_DestState == RES_LOADED)
            {
                pResource->SetState(result ? RES_LOADED : RES_FAILED);
                return;
            }
        }
    }
    */
}

//called by thread pool
void ResourceReloadCallback(void* pParam, int Instance, int ThreadID)
{
    ResourceBase* pResource = (ResourceBase*)pParam;

    if (InterlockedCompareExchange(&pResource->mState, RES_UNLOADING, RES_LOADED) == RES_LOADED)
    {
        LOG_INFO("Resource file '%s' modified", pResource->mName);

        pResource->OnUnload();
        pResource->SetState(RES_LOADING);
        bool result = pResource->OnLoad();
        pResource->SetState(result ? RES_LOADED : RES_FAILED);
    }
}


void ResManager::LoadResource(ResourceBase* pResource)
{
    InterlockedExchange(&pResource->mDestState, RES_LOADED);
    mThreadPool.AddTask(ResourceLoadingCallback, pResource, 1);
}

void ResManager::UnloadResource(ResourceBase* pResource)
{
    InterlockedExchange(&pResource->mDestState, RES_UNLOADED);
    mThreadPool.AddTask(ResourceUnloadingCallback, pResource, 1);
}

void ResManager::ReloadResource(ResourceBase* pResource)
{
    if (pResource->GetState() == RES_LOADED)
        mThreadPool.AddTask(ResourceReloadCallback, pResource, 1);
}

} // namespace Resource
} // namespace NFE

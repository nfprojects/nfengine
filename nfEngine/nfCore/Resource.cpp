/**
 * @file   Resource.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource base class definition.
 */

#include "stdafx.hpp"
#include "Globals.hpp"
#include "Resource.hpp"
#include "ResourcesManager.hpp"
#include "Memory.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Resource {

ResourceBase::ResourceBase()
{
    mCustom = false;
    mRefCount = 0;
    mState = RES_UNLOADED;
    mDestState = RES_DEFAULT;

    mOnLoad = NULL;
    mOnUnload = NULL;
    mUserPtr = NULL;
}

const char* ResourceBase::GetName() const
{
    return mName;
}

void ResourceBase::SetUserPointer(void* pPtr)
{
    mUserPtr = pPtr;
}

void* ResourceBase::GetUserPointer() const
{
    return mUserPtr;
}

Result ResourceBase::SetCallbacks(OnLoadCallback onLoadCallback,
                                  OnUnloadCallback onUnloadCallback)
{
#ifdef _DEBUG
    // check if passed pointers are function pointers

    if (onLoadCallback)
        if ((Util::MemoryCheck(onLoadCallback, 1) & ACCESS_EXECUTE) == 0)
        {
            LOG_ERROR("onLoadCallback does not point to executable memory region");
            return Result::CorruptedPointer;
        }

    if (onUnloadCallback)
        if ((Util::MemoryCheck(onUnloadCallback, 1) & ACCESS_EXECUTE) == 0)
        {
            LOG_ERROR("onUnloadCallback does not point to executable memory region");
            return Result::CorruptedPointer;
        }
#endif

    mOnLoad = onLoadCallback;
    mOnUnload = onUnloadCallback;
    return Result::OK;
}

bool ResourceBase::Rename(const char* pNewName)
{
    //check if name is not too long
    size_t nameLenght = strlen(pNewName);
    if (nameLenght >= RES_NAME_MAX_LENGTH)
    {
        LOG_ERROR("Resource name is to long (%i chars). Maximum is %i characters.", nameLenght,
                  RES_NAME_MAX_LENGTH - 1);
        return false;
    }

    //check if name is not already used
    if (g_pResManager->mResources.count(pNewName) > 0)
    {
        LOG_ERROR("Can not rename resource '%s' to '%s'. Name already used.", mName, pNewName);
        return false;
    }

    //erease old name
    g_pResManager->mResources.erase(mName);

    //add new entry with new name to resources map
    std::pair<const char*, ResourceBase*> resPair;
    resPair.first = pNewName;
    resPair.second = this;
    g_pResManager->mResources.insert(resPair);


    // ensure null-filled memory after the string
    ZeroMemory(mName, RES_NAME_MAX_LENGTH);

    // change name
    strcpy(mName, pNewName);

    return true;
}

uint32 ResourceBase::GetState() const
{
    return mState;
}

void ResourceBase::SetState(uint32 newState)
{
    InterlockedExchange(&mState, newState);
}

void ResourceBase::AddRef(void* ptr)
{
    if (InterlockedIncrement(&mRefCount) == 1)
        g_pResManager->LoadResource(this);
}

void ResourceBase::DelRef(void* ptr)
{
    if (mRefCount == 0)
    {
        LOG_ERROR("'%s' resource reference counting failed.", mName);
        return;
    }

    if (InterlockedDecrement(&mRefCount) == 0)
        g_pResManager->UnloadResource(this);
}

void ResourceBase::Load()
{
    if (InterlockedCompareExchange(&mState, RES_LOADING, RES_UNLOADED) == RES_UNLOADED)
    {
        //destination status changed during task waiting
        if (mDestState == RES_UNLOADED)
        {
            SetState(RES_UNLOADED);
            return;
        }

        for (;;)
        {
            bool result = OnLoad();

            if (mDestState != RES_UNLOADED)
            {
                SetState(result ? RES_LOADED : RES_FAILED);
                return;
            }

            //destination status changed during resource loading - unload resource
            SetState(RES_UNLOADING);
            OnUnload();

            if (mDestState == RES_UNLOADED)
            {
                SetState(RES_UNLOADED);
                return;
            }
        }
    }
}

void ResourceBase::Unload()
{
    if (InterlockedCompareExchange(&mState, RES_UNLOADING, RES_LOADED) == RES_LOADED)
    {
        //destination status changed during task waiting
        if (mDestState == RES_LOADED)
        {
            SetState(RES_LOADED);
            return;
        }

        for (;;)
        {
            OnUnload();
            if (mDestState != RES_LOADED)
            {
                SetState(RES_UNLOADED);
                return;
            }

            //destination status changed during resource loading - load resource back
            SetState(RES_LOADING);
            bool result = OnLoad();

            if (mDestState == RES_LOADED)
            {
                SetState(result ? RES_LOADED : RES_FAILED);
                return;
            }
        }
    }
}

} // namespace Resource
} // namespace NFE

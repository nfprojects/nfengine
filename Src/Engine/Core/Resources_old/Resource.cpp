/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource base class definition.
 */

#include "PCH.hpp"
#include "Engine.hpp"
#include "Resource.hpp"


namespace NFE {
namespace Resource {

using namespace Common;

ResourceBase::ResourceBase()
{
    mCustom = false;
    mRefCount = 0;
    mState = ResourceState::Unloaded;
    mDestState = ResourceState::Default;
    mFuncID = 0;

    mOnLoad = nullptr;
    mOnUnload = nullptr;
    mUserPtr = nullptr;

    memset(mName, '\0', sizeof(mName));
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

bool ResourceBase::SetCallbacks(OnLoadCallback onLoadCallback,
                                OnUnloadCallback onUnloadCallback)
{
    mOnLoad = onLoadCallback;
    mOnUnload = onUnloadCallback;
    return true;
}

bool ResourceBase::Rename(const char* pNewName)
{
    // check if name is not too long
    size_t nameLenght = strlen(pNewName);
    if (nameLenght >= RES_NAME_MAX_LENGTH)
    {
        NFE_LOG_ERROR("Resource name is to long (%i chars). Maximum is %i characters.", nameLenght,
                  RES_NAME_MAX_LENGTH - 1);
        return false;
    }

    ResManager* rm = Engine::GetInstance()->GetResManager();

    // check if name is not already used
    if (rm->mResources.count(pNewName) > 0)
    {
        NFE_LOG_ERROR("Can not rename resource '%s' to '%s'. Name already used.", mName, pNewName);
        return false;
    }

    // erease old name
    rm->mResources.erase(mName);

    // add new entry with new name to resources map
    std::pair<const char*, ResourceBase*> resPair;
    resPair.first = pNewName;
    resPair.second = this;
    rm->mResources.insert(resPair);

    // ensure null-filled memory after the string
    memset(mName, 0, RES_NAME_MAX_LENGTH);

    // change name
    strcpy(mName, pNewName);

    return true;
}

ResourceState ResourceBase::GetState() const
{
    return mState.load();
}

void ResourceBase::SetState(ResourceState newState)
{
    mState.store(newState);
}

void ResourceBase::AddRef(void* ptr)
{
    if (++mRefCount == 1)
    {
        ResManager* rm = Engine::GetInstance()->GetResManager();
        rm->LoadResource(this);
    }
}

void ResourceBase::DelRef(void* ptr)
{
    if (mRefCount.load() == 0)
    {
        NFE_LOG_ERROR("'%s' resource reference counting failed.", mName);
        return;
    }

    if (--mRefCount == 0)
    {
        ResManager* rm = Engine::GetInstance()->GetResManager();
        rm->UnloadResource(this);
    }
}

void ResourceBase::Load()
{
    ResourceState expected = ResourceState::Unloaded;
    mState.compare_exchange_strong(expected, ResourceState::Loading);
    if (expected == ResourceState::Unloaded)
    {
        // destination status changed during task waiting
        if (mDestState == ResourceState::Unloaded)
        {
            SetState(ResourceState::Unloaded);
            return;
        }

        for (;;)
        {
            bool result = OnLoad();

            if (mDestState != ResourceState::Unloaded)
            {
                SetState(result ? ResourceState::Loaded : ResourceState::Failed);

                // TODO get rid of that
                ScopedExclusiveLock<Mutex> lock(mCallbacksMutex);
                for (const auto& callback : mPostLoadCallbacks)
                    callback();
                mPostLoadCallbacks.clear();

                return;
            }

            // destination status changed during resource loading - unload resource
            SetState(ResourceState::Unloading);
            OnUnload();

            if (mDestState == ResourceState::Unloaded)
            {
                SetState(ResourceState::Unloaded);
                return;
            }
        }
    }
}

void ResourceBase::Unload()
{
    ResourceState expected = ResourceState::Loaded;
    mState.compare_exchange_strong(expected, ResourceState::Unloading);
    if (expected == ResourceState::Loaded)
    {
        // destination status changed during task waiting
        if (mDestState == ResourceState::Loaded)
        {
            SetState(ResourceState::Loaded);
            return;
        }

        for (;;)
        {
            OnUnload();
            if (mDestState != ResourceState::Loaded)
            {
                SetState(ResourceState::Unloaded);
                return;
            }

            // destination status changed during resource loading - load resource back
            SetState(ResourceState::Loading);
            bool result = OnLoad();

            if (mDestState == ResourceState::Loaded)
            {
                SetState(result ? ResourceState::Loaded : ResourceState::Failed);
                return;
            }
        }
    }
}

void ResourceBase::AddPostLoadCallback(const ResourcePostLoadCallback& callback)
{
    ScopedExclusiveLock<Mutex> lock(mCallbacksMutex);
    mPostLoadCallbacks.push_back(callback);

    // resource is already loaded, so call the callback
    if (mState == ResourceState::Loaded)
        callback();
}

} // namespace Resource
} // namespace NFE

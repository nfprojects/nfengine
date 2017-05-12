/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource base class definition.
 */

#include "PCH.hpp"
#include "Engine.hpp"
#include "Resource.hpp"
#include "ResourceManager.hpp"

#include "nfCommon/System/Memory.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/Utils/AsyncThreadPool.hpp"
#include "nfCommon/Utils/ScopedLock.hpp"

NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Resource::ResourceBase)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Resource {

ResourceBase::ResourceBase()
{
    mOnLoad = nullptr;
    mOnUnload = nullptr;
    mUserPtr = nullptr;
}

ResourceBase::~ResourceBase()
{
    // TODO notify the resource manager
}

/*
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
        ResourceManager* rm = Engine::GetInstance()->GetResManager();
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
        ResourceManager* rm = Engine::GetInstance()->GetResManager();
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
                Common::ScopedMutexLock lock(mCallbacksMutex);
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
    Common::ScopedMutexLock lock(mCallbacksMutex);
    mPostLoadCallbacks.push_back(callback);

    // resource is already loaded, so call the callback
    if (mState == ResourceState::Loaded)
        callback();
}

*/

} // namespace Resource
} // namespace NFE

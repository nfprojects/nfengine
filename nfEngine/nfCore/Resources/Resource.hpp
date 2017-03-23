/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource base class declarations.
 */

#pragma once

#include "../Core.hpp"

#include "nfCommon/Memory/Aligned.hpp"


namespace NFE {
namespace Resource {

/**
 * Resource states
 */
enum class ResourceState : int
{
    Default,
    Unloaded,
    Loading,
    Loaded,
    Unloading,
    Failed     /* resource failed to load, don't try it anymore */
};

/**
 * Resources supported by the engine.
 */
enum class ResourceType
{
    Unknown,
    Shader,
    Texture,
    Material,
    Mesh,
    CollisionShape,
    Sound,
};

// Maximum resource's name length in bytes (including terminating null)
#define RES_NAME_MAX_LENGTH 128


typedef bool (*OnLoadCallback)(ResourceBase*, void*);
typedef bool (*OnUnloadCallback)(ResourceBase*, void*);

typedef std::function<void()> ResourcePostLoadCallback;

/**
 * Base resource class.
 * @details Abstract resource class. It's main role is reference counter tracking and
            loading/unloading data when needed.
*/
class CORE_API ResourceBase
    : public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(ResourceBase)
    NFE_MAKE_NONMOVEABLE(ResourceBase)

    friend class ResManager;
    friend void ResourceLoadingCallback(void*, int, int);
    friend void ResourceUnloadingCallback(void*, int, int);
    friend void ResourceReloadCallback(void*, int, int);

private:
    // TODO: temporary hack
    std::mutex mCallbacksMutex;
    std::vector<ResourcePostLoadCallback> mPostLoadCallbacks;

protected:
    bool mCustom;                          // custom mesh won't be loaded from a file
    std::atomic<ResourceState> mState;     // current state
    std::atomic<ResourceState> mDestState; // destination state
    std::atomic<uint32> mRefCount;         // reference counter
    Common::AsyncFuncID mFuncID;           // loading/unloading function ID
    char mName[RES_NAME_MAX_LENGTH];       // resource name

    void* mUserPtr;
    OnLoadCallback mOnLoad;
    OnUnloadCallback mOnUnload;

    virtual bool OnLoad() = 0;
    virtual void OnUnload() = 0;
    void SetState(ResourceState newState);

public:
    ResourceBase();
    virtual ~ResourceBase() {};

    void SetUserPointer(void* pPtr);
    void* GetUserPointer() const;

    bool SetCallbacks(OnLoadCallback onLoadCallback = nullptr,
                      OnUnloadCallback onUnloadCallback = nullptr);

    const char* GetName() const;
    ResourceState GetState() const;

    /**
     * Increase reference counter. Should be used only in special cases.
     * @param ptr Custom user data used for debugging purposes.
     */
    void AddRef(void* ptr = nullptr);

    /**
     * Decrease reference counter. Should be used only in special cases.
     * @param ptr Custom user data used for debugging purposes.
     */
    void DelRef(void* ptr = nullptr);

    /**
     * Change resource's name
     * @param pNewName pNewName must be valid null-terminated string.
     */
    bool Rename(const char* pNewName);

    /**
     * Force to load resource
     */
    void Load();

    /**
     * Force to unload resource
     */
    void Unload();

    void AddPostLoadCallback(const ResourcePostLoadCallback& callback);
};

} // namespace Resource
} // namespace NFE

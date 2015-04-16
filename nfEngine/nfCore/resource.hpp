/**
 * @file   Resource.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource base class declarations.
 */

#pragma once

#include "Core.hpp"
#include "Aligned.hpp"

namespace NFE {
namespace Resource {

/// resource states
#define RES_DEFAULT     0
#define RES_UNLOADED    1
#define RES_LOADING     2
#define RES_LOADED      3
#define RES_UNLOADING   4
#define RES_FAILED      5 /* resource failed to load, don't try it anymore */

/**
 * Resources supported by the engine.
 */
enum class ResourceType
{
    Unknown,
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

/**
 * Base resource class.
 * @details Abstract resource class. It's main role is reference counter tracking and
            loading/unloading data when needed.
*/
class CORE_API ResourceBase : public Util::Aligned
{
    friend class ResManager;
    friend void ResourceLoadingCallback(void*, int, int);
    friend void ResourceUnloadingCallback(void*, int, int);
    friend void ResourceReloadCallback(void*, int, int);

    /// disable unwanted methods
    ResourceBase(const ResourceBase&);
    ResourceBase& operator= (const ResourceBase&);

protected:
    bool mCustom;                  // custom mesh won't be loaded from a file
    volatile uint32 mState;        // current state
    volatile uint32 mDestState;
    volatile uint32 mRefCount;     // reference counter TODO: references tracking in debug mode
    char mName[RES_NAME_MAX_LENGTH]; // resource name

    void* mUserPtr;
    OnLoadCallback mOnLoad;
    OnUnloadCallback mOnUnload;

    virtual bool OnLoad() = 0;
    virtual void OnUnload() = 0;
    void SetState(uint32 newState);

public:
    ResourceBase();
    virtual ~ResourceBase() {};

    void SetUserPointer(void* pPtr);
    void* GetUserPointer() const;

    Result SetCallbacks(OnLoadCallback onLoadCallback = NULL,
                        OnUnloadCallback onUnloadCallback = NULL);

    const char* GetName() const;
    uint32 GetState() const;

    /**
     * Increase reference counter. Should be used only in special cases.
     * @param ptr Custom user data used for debugging purposes.
     */
    void AddRef(void* ptr = NULL);

    /**
     * Decrease reference counter. Should be used only in special cases.
     * @param ptr Custom user data used for debugging purposes.
     */
    void DelRef(void* ptr = NULL);

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
};

} // namespace Resource
} // namespace NFE

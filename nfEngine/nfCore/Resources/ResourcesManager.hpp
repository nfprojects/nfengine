/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource manager declarations.
 */

#pragma once

#include "../Core.hpp"
#include "Resource.hpp"


namespace NFE {
namespace Resource {

/**
 * Class used by std::map to sort resources by name.
 */
class CompareResName
{
public:
    bool operator()(const char* a, const char* b) const
    {
        return strcmp(a, b) < 0;
    }
};

/**
 * Resources manager.
 */
// TODO: provide more functions
class CORE_API ResManager final
{
    NFE_MAKE_NONCOPYABLE(ResManager)
    NFE_MAKE_NONMOVEABLE(ResManager)

    friend class ResourceBase;
    friend class Engine;

private:
    std::unique_ptr<Common::AsyncThreadPool> mThreadPool;

    Common::Mutex mResListMutex;
    std::map<const char*, ResourceBase*, CompareResName> mResources;

    void Release();
    void LoadResource(ResourceBase* resource);
    void UnloadResource(ResourceBase* resource);
    void ReloadResource(ResourceBase* resource);

public:
    ResManager();
    ~ResManager();

    /**
     * Obtain pointer to a resource by a name and a type.
     *
     * @param pName Resource name.
     * @param type  Resource type.
     * @param check If set to false and a resource does not exist, the resource object is created
     *              automatically.
     * @return      Valid resource pointer or NULL on failure (see logs for more information).
     */
    ResourceBase* GetResource(const char* name, ResourceType type, bool check = false);

    /**
     * Insert a resource object created by the user.
     */
    bool AddCustomResource(ResourceBase* resource, const char* name);

    /**
     * Delete a resource from map (if the resource has no references).
     * @return "true" if the resource was deleted.
     */
    bool DeleteResource(const char* name);

    /**
     * Wait for a resource to be loaded/unloaded.
     */
    bool WaitForResource(ResourceBase* resource);
};

} // namespace Resource
} // namespace NFE

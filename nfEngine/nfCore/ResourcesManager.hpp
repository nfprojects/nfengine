/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource manager declarations.
 */

#pragma once

#include "Core.hpp"
#include "Resource.hpp"
#include "../nfCommon/ThreadPool.hpp"

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
class CORE_API ResManager
{
    friend class ResourceBase;

    //disable these:
    ResManager(const ResManager&);
    ResManager& operator= (const ResManager&);

    std::unique_ptr<Common::ThreadPool> mThreadPool;

    std::mutex mResListMutex;
    std::map<const char*, ResourceBase*, CompareResName> mResources;

    void LoadResource(ResourceBase* pResource);
    void UnloadResource(ResourceBase* pResource);
    void ReloadResource(ResourceBase* pResource);

public:
    ResManager();
    ~ResManager();

    /**
     * Obtain pointer to a resource by a name.
     * @param pName Resource name.
     * @param type Resource type.
     * @param check If set to false and a resource does not exist, the resource object is created automatically.
     * @return Valid resource pointer or NULL on failure (see logs for more information).
     */
    ResourceBase* GetResource(const char* pName, ResourceType type, bool check = false);

    /**
     * Insert resource object created by the user.
     */
    Result AddCustomResource(ResourceBase* pResource, const char* pName);


    int DeleteResource(const char* pName);
    bool IsBusy();

    // called when a file in "\Data" directory was changed
    void OnFileChanged(const char* pFile);
};

} // namespace Resource
} // namespace NFE

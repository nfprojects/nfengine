/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource base class declarations.
 */

#pragma once

#include "../Core.hpp"

#include "nfCommon/Containers/String.hpp"
#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Reflection/ReflectionMacros.hpp"

#include <functional>

namespace NFE {
namespace Resource {


typedef bool (*OnLoadCallback)(ResourceBase*, void*);
typedef bool (*OnUnloadCallback)(ResourceBase*, void*);

class ResourceBase;
using ResourcePtr = std::shared_ptr<ResourceBase>;
using ResourceWeakPtr = std::weak_ptr<ResourceBase>;
using ResourceName = Common::String;

/**
 * Base resource class. Represents a LOADED resource.
 */
class CORE_API ResourceBase
    : public Common::Aligned<16>
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ResourceBase)
    NFE_MAKE_NONCOPYABLE(ResourceBase)

    friend class ResourceManager;
    friend void ResourceLoadingCallback(void*, int, int);
    friend void ResourceUnloadingCallback(void*, int, int);
    friend void ResourceReloadCallback(void*, int, int);

public:
    ResourceBase();
    virtual ~ResourceBase();

    const ResourceName& GetName() const { return mName; }

protected:
    void* mUserPtr;
    OnLoadCallback mOnLoad;
    OnUnloadCallback mOnUnload;

    /**
     * Load the resource.
     * @note    Called by the resource loading task.
     */
    virtual bool OnLoad() = 0;

    /**
     * Unload the resource.
     * @note    Called by the resource manager.
     */
    virtual void OnUnload() = 0;

private:
    ResourceName mName;
};

} // namespace Resource
} // namespace NFE

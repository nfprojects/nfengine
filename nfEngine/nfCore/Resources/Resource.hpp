/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Resource base class declarations.
 */

#pragma once

#include "../Core.hpp"
#include "ResourceName.hpp"

#include "nfCommon/Containers/String.hpp"
#include "nfCommon/Reflection/ReflectionClassMacros.hpp"

#include <functional>


namespace NFE {
namespace Resource {

class ResourceBase;
using ResourcePtr = Common::SharedPtr<ResourceBase>;
using ResourceWeakPtr = Common::WeakPtr<ResourceBase>;

/**
 * Base resource class. Represents a LOADED resource.
 */
class CORE_API ResourceBase
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ResourceBase)
    NFE_MAKE_NONCOPYABLE(ResourceBase)

public:
    ResourceBase();
    virtual ~ResourceBase();

    const ResourceName& GetName() const { return mName; }

protected:

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

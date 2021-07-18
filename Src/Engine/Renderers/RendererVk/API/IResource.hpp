/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan renderer's IResource base class
 */

#pragma once

#include "../RendererCommon/ResourceBinding.hpp"
#include "Engine/Common/Containers/SharedPtr.hpp"
#include "Internal/Types.hpp"


namespace NFE {
namespace Renderer {

// Base Resource interface, used for easier resource tracking
class IResource
{
protected:
    Internal::ResourceID mID;

public:
    virtual const Internal::ResourceType GetType() const = 0;

    NFE_INLINE Internal::ResourceID GetID() const
    {
        return mID;
    }
};

using ResourcePtr = Common::SharedPtr<IResource>;

} // namespace Renderer
} // namespace NFE

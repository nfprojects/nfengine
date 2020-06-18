/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of Vulkan renderer's IResource base class
 */

#pragma once

#include "../Interface/ResourceBinding.hpp"


namespace NFE {
namespace Renderer {

// Base Resource interface, used for easier resource tracking
class IResource
{
public:
    virtual const ShaderResourceType GetType() const = 0;
};

} // namespace Renderer
} // namespace NFE

/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Win32-specific Vulkan extensions
 */

#pragma once

#include "../Defines.hpp"

namespace NFE {
namespace Renderer {

extern PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

/**
 * Initializes Win32Surface-related device extensions.
 */
bool nfvkWin32InstanceExtensionsInit(VkInstance instance);

} // namespace Renderer
} // namespace NFE

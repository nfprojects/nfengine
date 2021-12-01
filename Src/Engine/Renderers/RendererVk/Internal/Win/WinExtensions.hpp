/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declarations of Windows-specific Vulkan extensions
 */

#pragma once

#include "Defines.hpp"

namespace NFE {
namespace Renderer {

extern PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

/**
 * Initializes Win32Surface-related device extensions.
 */
bool nfvkWinInstanceExtensionsInit(VkInstance instance);

} // namespace Renderer
} // namespace NFE

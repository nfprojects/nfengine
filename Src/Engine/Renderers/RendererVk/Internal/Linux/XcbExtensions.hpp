/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of XCB-specific Vulkan extensions
 */

#pragma once

#include "Defines.hpp"

namespace NFE {
namespace Renderer {

extern PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR;

/**
 * Initializes Xcb Surface-related device extensions.
 */
bool nfvkXcbInstanceExtensionsInit(VkInstance instance);

} // namespace Renderer
} // namespace NFE

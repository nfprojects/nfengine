/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan extensions
 */

#pragma once


namespace NFE {
namespace Renderer {

// Instance extensions
// If any useful Device-related extensions show up outside of VK Core, they should go in here

/**
 * Initializes Vulkan Instance extensions used by nfRendererVk.
 *
 * @param  Valid Instance object.
 * @return True if all extensions were successfully acquired, false otherwise.
 */
bool nfvkInstanceExtensionsInit(VkInstance instance);


// Device extensions
// If any useful Device-related extensions show up outside of VK Core, they should go in here

/**
 * Initializes Vulkan Device extensions used by nfRendererVk.
 *
 * @param  Valid Device object.
 * @return True if all extensions were successfully acquired, false otherwise.
 */
bool nfvkDeviceExtensionsInit(VkDevice device);

} // namespace Renderer
} // namespace NFE

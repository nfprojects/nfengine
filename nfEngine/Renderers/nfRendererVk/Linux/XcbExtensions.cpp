/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of XCB-specific Vulkan extensions
 */

#include "../PCH.hpp"

#include "XcbExtensions.hpp"

#include "../Extensions.hpp"
#include "../GetProc.hpp"


namespace NFE {
namespace Renderer {

PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR = VK_NULL_HANDLE;

// Initializes only XCB-specific extensions
bool nfvkXcbInstanceExtensionsInit(VkInstance instance)
{
    bool allExtensionsAvailable = true;

    VK_GET_INSTANCEPROC(instance, vkCreateXcbSurfaceKHR);

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE

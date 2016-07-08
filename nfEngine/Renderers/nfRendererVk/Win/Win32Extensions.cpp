/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Win32-specific Vulkan extensions
 */

#include "../PCH.hpp"

#include "Win32Extensions.hpp"

#include "../Extensions.hpp"
#include "../GetProc.hpp"


namespace NFE {
namespace Renderer {

PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = VK_NULL_HANDLE;

// Initializes only Win32-specific extensions
bool nfvkWin32InstanceExtensionsInit(VkInstance instance)
{
    bool allExtensionsAvailable = true;

    VK_GET_INSTANCEPROC(instance, vkCreateWin32SurfaceKHR);

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE

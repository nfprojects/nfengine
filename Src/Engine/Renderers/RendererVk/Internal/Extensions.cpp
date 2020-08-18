/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan extensions used in functions
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Extensions.hpp"

#include "GetProc.hpp"

#include "Engine/Common/Logger/Logger.hpp"

#ifdef NFE_PLATFORM_WINDOWS
#include "Win/WinExtensions.hpp"
#elif defined(NFE_PLATFORM_LINUX)
#include "Linux/XcbExtensions.hpp"
#else
#error "Target platform not supported."
#endif


namespace NFE {
namespace Renderer {

bool nfvkInstanceExtensionsInit(VkInstance instance)
{
    bool allExtensionsAvailable = true;

#ifdef NFE_PLATFORM_WINDOWS
    if (!nfvkWinInstanceExtensionsInit(instance))
        NFE_LOG_WARNING("Win32 extensions failed to init, some Vulkan Win32 APIs might be unavailable.");
#elif defined(NFE_PLATFORM_LINUX)
    if (!nfvkXcbInstanceExtensionsInit(instance))
        NFE_LOG_WARNING("Xcb extensions failed to init, some Vulkan Xcb APIs might be unavailable.");
#else
#error Invalid platform
#endif

    return allExtensionsAvailable;
}


bool nfvkDeviceExtensionsInit(VkDevice device)
{
    bool allExtensionsAvailable = true;

    NFE_UNUSED(device);

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE

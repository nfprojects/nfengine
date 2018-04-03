/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan extensions used in functions
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Extensions.hpp"

#include "GetProc.hpp"

#include "nfCommon/Logger/Logger.hpp"

#ifdef WIN32
#include "Win/WinExtensions.hpp"
#elif defined(__linux__) | defined(__LINUX__)
#include "Linux/XcbExtensions.hpp"
#else
#error "Target platform not supported."
#endif


namespace NFE {
namespace Renderer {

bool nfvkInstanceExtensionsInit(VkInstance instance)
{
    bool allExtensionsAvailable = true;

#ifdef WIN32
    if (!nfvkWinInstanceExtensionsInit(instance))
        NFE_LOG_WARNING("Win32 extensions failed to init, some Vulkan Win32 APIs might be unavailable.");
#elif defined(__linux__) | defined(__LINUX__)
    if (!nfvkXcbInstanceExtensionsInit(instance))
        NFE_LOG_WARNING("Xcb extensions failed to init, some Vulkan Xcb APIs might be unavailable.");
#else
#error "Target platform not supported."
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

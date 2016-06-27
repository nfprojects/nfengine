/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan extensions used in functions
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Extensions.hpp"

#include "GetExtension.hpp"


namespace NFE {
namespace Renderer {


bool nfvkExtensionsInit()
{
    bool allExtensionsAvailable = true;

#ifdef WIN32
    if (!nfvkWin32ExtensionsInit())
        LOG_WARNING("Win32 extensions failed to init, some Vulkan Win32 APIs might be unavailable.");
#elif defined(__linux__) | defined(__LINUX__)
    if (!nfvkXlibExtensionsInit())
        LOG_WARNING("Xlib extensions failed to init, some Vulkan Xlib APIs might be unavailable.");
#else
#error "Target platform not supported."
#endif

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE

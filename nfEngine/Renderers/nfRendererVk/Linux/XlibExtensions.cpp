/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Linux-specific Vulkan extensions
 */

#include "../PCH.hpp"

#include "XlibExtensions.hpp"

#include "../GetExtension.hpp"
#include "Logger.hpp"


namespace NFE {
namespace Renderer {

// Initializes only GLX-specific extensions
bool nfglxExtensionsInit()
{
    bool allExtensionsAvailable = true;

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE

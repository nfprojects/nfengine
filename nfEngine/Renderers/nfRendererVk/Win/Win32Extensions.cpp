/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Win32-specific Vulkan extensions
 */

#include "../PCH.hpp"

#include "Win32Extensions.hpp"

#include "../GetExtension.hpp"


namespace NFE {
namespace Renderer {

// Initializes only Win32-specific extensions
bool nfvkWin32ExtensionsInit()
{
    bool allExtensionsAvailable = true;

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE

/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Linux-specific OpenGL extensions
 */

#include "../PCH.hpp"

#include "glXExtensions.hpp"

#include "../GetExtension.hpp"
#include "Logger.hpp"


namespace NFE {
namespace Renderer {

PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = nullptr;

// Initializes only GLX-specific extensions
bool nfglxExtensionsInit()
{
    bool allExtensionsAvailable = true;

    NFGL_GET_EXTENSION(PFNGLXSWAPINTERVALEXTPROC, glXSwapIntervalEXT);

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE

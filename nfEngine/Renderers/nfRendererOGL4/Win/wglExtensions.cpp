/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of Windows-specific OpenGL extensions
 */

#include "../PCH.hpp"

#include "wglExtensions.hpp"

#include "../GetExtension.hpp"


namespace NFE {
namespace Renderer {

PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;
PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = nullptr;

// Initializes only WGL-specific extensions
bool nfwglExtensionsInit()
{
    bool allExtensionsAvailable = true;

    NFGL_GET_EXTENSION(PFNWGLSWAPINTERVALEXTPROC, wglSwapIntervalEXT);
    NFGL_GET_EXTENSION(PFNWGLGETSWAPINTERVALEXTPROC, wglGetSwapIntervalEXT);

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE

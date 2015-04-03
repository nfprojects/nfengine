/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of OpenGL extensions used in functions
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Extensions.hpp"

// function pointers
wglSwapIntervalEXTType wglSwapIntervalEXT = nullptr;
wglGetSwapIntervalEXTType wglGetSwapIntervalEXT = nullptr;

/**
 * Get an OpenGL extension using wglGetProcAddress.
 *
 * wglGetProcAddress returns a nullptr when the extension is not available. When such situation
 * occurs, appropriate information is logged and allExtensionsAvailable is set to false.
 */
#define NFGL_GET_EXTENSION(x)                                                               \
do {                                                                                        \
    x = reinterpret_cast<x##Type>(wglGetProcAddress(#x));                                   \
    if (!x)                                                                                 \
    {                                                                                       \
        LOG_ERROR("Unable to retrieve " #x " extension function: %lu.", GetLastError());    \
        allExtensionsAvailable = false;                                                     \
    }                                                                                       \
} while(0)

namespace NFE {
namespace Renderer {

bool nfglExtensionsInit()
{
    bool allExtensionsAvailable = true;

    NFGL_GET_EXTENSION(wglSwapIntervalEXT);
    NFGL_GET_EXTENSION(wglGetSwapIntervalEXT);

    return allExtensionsAvailable;
}

} // namespace Renderer
} // namespace NFE

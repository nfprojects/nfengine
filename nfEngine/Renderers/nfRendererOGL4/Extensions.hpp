/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of OpenGL extensions
 */

#pragma once

// TODO The implementation defined here is WINDOWS only.
//      At some point, some extensions will become platform-specific.

/// Function pointer typedefs
typedef BOOL (__stdcall *wglSwapIntervalEXTType)(int interval);
typedef int (__stdcall *wglGetSwapIntervalEXTType)(void);

/// Function pointers
extern wglSwapIntervalEXTType wglSwapIntervalEXT;
extern wglGetSwapIntervalEXTType wglGetSwapIntervalEXT;

namespace NFE {
namespace Renderer {

/**
 * Initializes OpenGL extensions used by nfRendererOGL4.
 *
 * The function performs all the initialization work needed to enable extensions used by OpenGL
 * implementation of nfRenderer. Failing to get any of the extensions will result in an error.
 *
 * @return True if all extensions were successfully acquired, false otherwise.
 */
bool nfglExtensionsInit();

} // namespace Renderer
} // namespace NFE

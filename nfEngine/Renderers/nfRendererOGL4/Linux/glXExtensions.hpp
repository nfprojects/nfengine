/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Linux-specific OpenGL extensions
 */

#pragma once

#include "../Defines.hpp"
#include "../GL/glxext.h"

// GLX functions are context-independent. Directly defining their name might cause SIGSEGV because
// of overriding already used existing pointers. Instead of directly naming the functions, add
// a "p" letter as a prefix, and then create a macro to use the correct name in-code.
extern PFNGLXSWAPINTERVALEXTPROC pglXSwapIntervalEXT;
#define glXSwapIntervalEXT pglXSwapIntervalEXT

namespace NFE {
namespace Renderer {

// Initializes only GLX-specific extensions
bool nfglxExtensionsInit();

} // namespace Renderer
} // namespace NFE

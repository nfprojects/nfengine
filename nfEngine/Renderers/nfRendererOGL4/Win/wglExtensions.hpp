/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Linux-specific OpenGL extensions
 */

#pragma once

#include "../Defines.hpp"
#include "../GL/wglext.h"

extern PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
extern PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;

namespace NFE {
namespace Renderer {

// Initializes only GLX-specific extensions
bool nfwglExtensionsInit();

} // namespace Renderer
} // namespace NFE

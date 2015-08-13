/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Linux-specific OpenGL extensions
 */

#pragma once

#include "../Defines.hpp"
#include "GL/wglext.h"


namespace NFE {
namespace Renderer {

extern PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
extern PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;

// Initializes only GLX-specific extensions
bool nfwglExtensionsInit();

} // namespace Renderer
} // namespace NFE

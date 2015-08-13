/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Linux-specific OpenGL extensions
 */

#pragma once

#include "../Defines.hpp"
#include "GL/glxext.h"


namespace NFE {
namespace Renderer {

extern PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;

// Initializes only GLX-specific extensions
bool nfglxExtensionsInit();

} // namespace Renderer
} // namespace NFE

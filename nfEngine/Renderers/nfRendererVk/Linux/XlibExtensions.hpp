/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Xlib-specific Vulkan extensions
 */

#pragma once

#include "../Defines.hpp"

namespace NFE {
namespace Renderer {

// Initializes only GLX-specific extensions
bool nfvkXlibExtensionsInit();

} // namespace Renderer
} // namespace NFE

/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Win32-specific Vulkan extensions
 */

#pragma once

#include "../Defines.hpp"

namespace NFE {
namespace Renderer {

// Initializes only Win32-specific extensions
bool nfvkWin32ExtensionsInit();

} // namespace Renderer
} // namespace NFE

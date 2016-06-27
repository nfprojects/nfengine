/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan extensions
 */

#pragma once

#ifdef WIN32
#include "Win/Win32Extensions.hpp"
#elif defined(__linux__) | defined(__LINUX__)
#include "Linux/XlibExtensions.hpp"
#else
#error "Target platform not supported."
#endif


namespace NFE {
namespace Renderer {

/**
 * Initializes Vulkan extensions used by nfRendererVk.
 *
 * The function performs all the initialization work needed to enable extensions used by OpenGL
 * implementation of nfRenderer. Failing to get any of the extensions will result in an error.
 *
 * @return True if all extensions were successfully acquired, false otherwise.
 */
bool nfvkExtensionsInit();

} // namespace Renderer
} // namespace NFE

/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan <-> nfEngine translations functions.
 */

#pragma once

#include "../RendererInterface/Types.hpp"

#include "Defines.hpp"

namespace NFE {
namespace Renderer {

const char* TranslateDeviceTypeToString(VkPhysicalDeviceType type);
VkFormat TranslateElementFormatToVkFormat(ElementFormat format);
const char* TranslateVkResultToString(VkResult result);

} // namespace Renderer
} // namespace NFE

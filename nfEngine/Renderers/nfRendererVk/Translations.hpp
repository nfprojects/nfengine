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

// debugging helpers
const char* TranslateDeviceTypeToString(VkPhysicalDeviceType type);
const char* TranslateVkResultToString(VkResult result);

// NFE to Vulkan
VkBlendFactor TranslateBlendFuncToVkBlendFactor(BlendFunc func);
VkBlendOp TranslateBlendOpToVkBlendOp(BlendOp op);
VkCompareOp TranslateCompareFuncToVkCompareOp(CompareFunc func);
VkCullModeFlags TranslateCullModeToVkCullMode(CullMode mode);
VkFormat TranslateElementFormatToVkFormat(ElementFormat format, int size);
uint32 TranslateElementFormatToByteSize(ElementFormat format);
VkPolygonMode TranslateFillModeToVkPolygonMode(FillMode mode);
VkPrimitiveTopology TranslatePrimitiveTypeToVkTopology(PrimitiveType type);
VkStencilOp TranslateStencilOpToVkStencilOp(StencilOp op);

} // namespace Renderer
} // namespace NFE

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
VkBufferUsageFlags TranslateBufferTypeToVkBufferUsage(BufferType type);
VkCompareOp TranslateCompareFuncToVkCompareOp(CompareFunc func);
VkCullModeFlags TranslateCullModeToVkCullMode(CullMode mode);
VkFormat TranslateDepthFormatToVkFormat(DepthBufferFormat format);
VkFormat TranslateElementFormatToVkFormat(ElementFormat format);
VkPolygonMode TranslateFillModeToVkPolygonMode(FillMode mode);
VkIndexType TranslateIndexBufferFormatToVkIndexType(IndexBufferFormat format);
VkPrimitiveTopology TranslatePrimitiveTypeToVkTopology(PrimitiveType type);
VkShaderStageFlagBits TranslateShaderTypeToVkShaderStage(ShaderType type);
VkStencilOp TranslateStencilOpToVkStencilOp(StencilOp op);

} // namespace Renderer
} // namespace NFE

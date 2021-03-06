/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan <-> nfEngine translations functions.
 */

#pragma once

#include "../RendererCommon/Types.hpp"
#include "../RendererCommon/ResourceBinding.hpp"

#include "Defines.hpp"

namespace NFE {
namespace Renderer {

// debugging helpers
const char* TranslateDeviceTypeToString(VkPhysicalDeviceType type);
const char* TranslateVkResultToString(VkResult result);
const char* TranslateVkFormatToString(VkFormat format);

// NFE to Vulkan
VkBlendFactor TranslateBlendFuncToVkBlendFactor(BlendFunc func);
VkBlendOp TranslateBlendOpToVkBlendOp(BlendOp op);
VkBufferUsageFlags TranslateBufferUsageToVkBufferUsage(BufferUsageFlag usage);
VkCompareOp TranslateCompareFuncToVkCompareOp(CompareFunc func);
VkCullModeFlags TranslateCullModeToVkCullMode(CullMode mode);
VkDescriptorType TranslateDynamicResourceTypeToVkDescriptorType(ShaderResourceType type);
VkFormat TranslateFormatToVkFormat(Format format);
VkPolygonMode TranslateFillModeToVkPolygonMode(FillMode mode);
VkIndexType TranslateIndexBufferFormatToVkIndexType(IndexBufferFormat format);
VkFilter TranslateMagFilterToVkFilter(TextureMagFilter filter);
VkFilter TranslateMinFilterToVkFilter(TextureMinFilter filter);
VkSamplerMipmapMode TranslateMinFilterToVkSamplerMipmapMode(TextureMinFilter filter);
VkPrimitiveTopology TranslatePrimitiveTypeToVkTopology(PrimitiveType type);
VkSampleCountFlagBits TranslateSamplesNumToVkSampleCount(uint32 sampleNum);
VkDescriptorType TranslateShaderResourceTypeToVkDescriptorType(ShaderResourceType type);
VkShaderStageFlagBits TranslateShaderTypeToVkShaderStage(ShaderType type);
VkStencilOp TranslateStencilOpToVkStencilOp(StencilOp op);
VkSamplerAddressMode TranslateWrapModeToVkSamplerAddressMode(TextureWrapMode mode);

} // namespace Renderer
} // namespace NFE

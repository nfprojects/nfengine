/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Definitions of Vulkan <-> nfEngine translations functions.
 */

#include "PCH.hpp"

#include "Translations.hpp"

namespace NFE {
namespace Renderer {

const char* TranslateDeviceTypeToString(VkPhysicalDeviceType type)
{
    switch (type)
    {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "OTHER";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "INTEGRATED_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "DISCRETE_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "VIRTUAL_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
    default: return "UNKNOWN";
    }
}

const char* TranslateVkResultToString(VkResult result)
{
    switch (result)
    {
    case VK_SUCCESS: return "SUCCESS";
    case VK_NOT_READY: return "NOT_READY";
    case VK_TIMEOUT: return "TIMEOUT";
    case VK_EVENT_SET: return "EVENT_SET";
    case VK_EVENT_RESET: return "EVENT_RESET";
    case VK_INCOMPLETE: return "INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return "ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST: return "ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED: return "ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT: return "ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT: return "ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return "ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS: return "ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED: return "ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_SURFACE_LOST_KHR: return "ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR: return "SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR: return "ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT: return "ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV: return "ERROR_INVALID_SHADER_NV";
    default: return "UNKNOWN";
    }
}

const char* TranslateVkFormatToString(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_UNDEFINED: return "UNDEFINED";
        case VK_FORMAT_R4G4_UNORM_PACK8: return "R4G4_UNORM_PACK8";
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return "R4G4B4A4_UNORM_PACK16";
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return "B4G4R4A4_UNORM_PACK16";
        case VK_FORMAT_R5G6B5_UNORM_PACK16: return "R5G6B5_UNORM_PACK16";
        case VK_FORMAT_B5G6R5_UNORM_PACK16: return "B5G6R5_UNORM_PACK16";
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return "R5G5B5A1_UNORM_PACK16";
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16: return "B5G5R5A1_UNORM_PACK16";
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return "A1R5G5B5_UNORM_PACK16";
        case VK_FORMAT_R8_UNORM: return "R8_UNORM";
        case VK_FORMAT_R8_SNORM: return "R8_SNORM";
        case VK_FORMAT_R8_USCALED: return "R8_USCALED";
        case VK_FORMAT_R8_SSCALED: return "R8_SSCALED";
        case VK_FORMAT_R8_UINT: return "R8_UINT";
        case VK_FORMAT_R8_SINT: return "R8_SINT";
        case VK_FORMAT_R8_SRGB: return "R8_SRGB";
        case VK_FORMAT_R8G8_UNORM: return "R8G8_UNORM";
        case VK_FORMAT_R8G8_SNORM: return "R8G8_SNORM";
        case VK_FORMAT_R8G8_USCALED: return "R8G8_USCALED";
        case VK_FORMAT_R8G8_SSCALED: return "R8G8_SSCALED";
        case VK_FORMAT_R8G8_UINT: return "R8G8_UINT";
        case VK_FORMAT_R8G8_SINT: return "R8G8_SINT";
        case VK_FORMAT_R8G8_SRGB: return "R8G8_SRGB";
        case VK_FORMAT_R8G8B8_UNORM: return "R8G8B8_UNORM";
        case VK_FORMAT_R8G8B8_SNORM: return "R8G8B8_SNORM";
        case VK_FORMAT_R8G8B8_USCALED: return "R8G8B8_USCALED";
        case VK_FORMAT_R8G8B8_SSCALED: return "R8G8B8_SSCALED";
        case VK_FORMAT_R8G8B8_UINT: return "R8G8B8_UINT";
        case VK_FORMAT_R8G8B8_SINT: return "R8G8B8_SINT";
        case VK_FORMAT_R8G8B8_SRGB: return "R8G8B8_SRGB";
        case VK_FORMAT_B8G8R8_UNORM: return "B8G8R8_UNORM";
        case VK_FORMAT_B8G8R8_SNORM: return "B8G8R8_SNORM";
        case VK_FORMAT_B8G8R8_USCALED: return "B8G8R8_USCALED";
        case VK_FORMAT_B8G8R8_SSCALED: return "B8G8R8_SSCALED";
        case VK_FORMAT_B8G8R8_UINT: return "B8G8R8_UINT";
        case VK_FORMAT_B8G8R8_SINT: return "B8G8R8_SINT";
        case VK_FORMAT_B8G8R8_SRGB: return "B8G8R8_SRGB";
        case VK_FORMAT_R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
        case VK_FORMAT_R8G8B8A8_SNORM: return "R8G8B8A8_SNORM";
        case VK_FORMAT_R8G8B8A8_USCALED: return "R8G8B8A8_USCALED";
        case VK_FORMAT_R8G8B8A8_SSCALED: return "R8G8B8A8_SSCALED";
        case VK_FORMAT_R8G8B8A8_UINT: return "R8G8B8A8_UINT";
        case VK_FORMAT_R8G8B8A8_SINT: return "R8G8B8A8_SINT";
        case VK_FORMAT_R8G8B8A8_SRGB: return "R8G8B8A8_SRGB";
        case VK_FORMAT_B8G8R8A8_UNORM: return "B8G8R8A8_UNORM";
        case VK_FORMAT_B8G8R8A8_SNORM: return "B8G8R8A8_SNORM";
        case VK_FORMAT_B8G8R8A8_USCALED: return "B8G8R8A8_USCALED";
        case VK_FORMAT_B8G8R8A8_SSCALED: return "B8G8R8A8_SSCALED";
        case VK_FORMAT_B8G8R8A8_UINT: return "B8G8R8A8_UINT";
        case VK_FORMAT_B8G8R8A8_SINT: return "B8G8R8A8_SINT";
        case VK_FORMAT_B8G8R8A8_SRGB: return "B8G8R8A8_SRGB";
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32: return "A8B8G8R8_UNORM_PACK32";
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32: return "A8B8G8R8_SNORM_PACK32";
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32: return "A8B8G8R8_USCALED_PACK32";
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: return "A8B8G8R8_SSCALED_PACK32";
        case VK_FORMAT_A8B8G8R8_UINT_PACK32: return "A8B8G8R8_UINT_PACK32";
        case VK_FORMAT_A8B8G8R8_SINT_PACK32: return "A8B8G8R8_SINT_PACK32";
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return "A8B8G8R8_SRGB_PACK32";
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return "A2R10G10B10_UNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return "A2R10G10B10_SNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32: return "A2R10G10B10_USCALED_PACK32";
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: return "A2R10G10B10_SSCALED_PACK32";
        case VK_FORMAT_A2R10G10B10_UINT_PACK32: return "A2R10G10B10_UINT_PACK32";
        case VK_FORMAT_A2R10G10B10_SINT_PACK32: return "A2R10G10B10_SINT_PACK32";
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return "A2B10G10R10_UNORM_PACK32";
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return "A2B10G10R10_SNORM_PACK32";
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32: return "A2B10G10R10_USCALED_PACK32";
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: return "A2B10G10R10_SSCALED_PACK32";
        case VK_FORMAT_A2B10G10R10_UINT_PACK32: return "A2B10G10R10_UINT_PACK32";
        case VK_FORMAT_A2B10G10R10_SINT_PACK32: return "A2B10G10R10_SINT_PACK32";
        case VK_FORMAT_R16_UNORM: return "R16_UNORM";
        case VK_FORMAT_R16_SNORM: return "R16_SNORM";
        case VK_FORMAT_R16_USCALED: return "R16_USCALED";
        case VK_FORMAT_R16_SSCALED: return "R16_SSCALED";
        case VK_FORMAT_R16_UINT: return "R16_UINT";
        case VK_FORMAT_R16_SINT: return "R16_SINT";
        case VK_FORMAT_R16_SFLOAT: return "R16_SFLOAT";
        case VK_FORMAT_R16G16_UNORM: return "R16G16_UNORM";
        case VK_FORMAT_R16G16_SNORM: return "R16G16_SNORM";
        case VK_FORMAT_R16G16_USCALED: return "R16G16_USCALED";
        case VK_FORMAT_R16G16_SSCALED: return "R16G16_SSCALED";
        case VK_FORMAT_R16G16_UINT: return "R16G16_UINT";
        case VK_FORMAT_R16G16_SINT: return "R16G16_SINT";
        case VK_FORMAT_R16G16_SFLOAT: return "R16G16_SFLOAT";
        case VK_FORMAT_R16G16B16_UNORM: return "R16G16B16_UNORM";
        case VK_FORMAT_R16G16B16_SNORM: return "R16G16B16_SNORM";
        case VK_FORMAT_R16G16B16_USCALED: return "R16G16B16_USCALED";
        case VK_FORMAT_R16G16B16_SSCALED: return "R16G16B16_SSCALED";
        case VK_FORMAT_R16G16B16_UINT: return "R16G16B16_UINT";
        case VK_FORMAT_R16G16B16_SINT: return "R16G16B16_SINT";
        case VK_FORMAT_R16G16B16_SFLOAT: return "R16G16B16_SFLOAT";
        case VK_FORMAT_R16G16B16A16_UNORM: return "R16G16B16A16_UNORM";
        case VK_FORMAT_R16G16B16A16_SNORM: return "R16G16B16A16_SNORM";
        case VK_FORMAT_R16G16B16A16_USCALED: return "R16G16B16A16_USCALED";
        case VK_FORMAT_R16G16B16A16_SSCALED: return "R16G16B16A16_SSCALED";
        case VK_FORMAT_R16G16B16A16_UINT: return "R16G16B16A16_UINT";
        case VK_FORMAT_R16G16B16A16_SINT: return "R16G16B16A16_SINT";
        case VK_FORMAT_R16G16B16A16_SFLOAT: return "R16G16B16A16_SFLOAT";
        case VK_FORMAT_R32_UINT: return "R32_UINT";
        case VK_FORMAT_R32_SINT: return "R32_SINT";
        case VK_FORMAT_R32_SFLOAT: return "R32_SFLOAT";
        case VK_FORMAT_R32G32_UINT: return "R32G32_UINT";
        case VK_FORMAT_R32G32_SINT: return "R32G32_SINT";
        case VK_FORMAT_R32G32_SFLOAT: return "R32G32_SFLOAT";
        case VK_FORMAT_R32G32B32_UINT: return "R32G32B32_UINT";
        case VK_FORMAT_R32G32B32_SINT: return "R32G32B32_SINT";
        case VK_FORMAT_R32G32B32_SFLOAT: return "R32G32B32_SFLOAT";
        case VK_FORMAT_R32G32B32A32_UINT: return "R32G32B32A32_UINT";
        case VK_FORMAT_R32G32B32A32_SINT: return "R32G32B32A32_SINT";
        case VK_FORMAT_R32G32B32A32_SFLOAT: return "R32G32B32A32_SFLOAT";
        case VK_FORMAT_R64_UINT: return "R64_UINT";
        case VK_FORMAT_R64_SINT: return "R64_SINT";
        case VK_FORMAT_R64_SFLOAT: return "R64_SFLOAT";
        case VK_FORMAT_R64G64_UINT: return "R64G64_UINT";
        case VK_FORMAT_R64G64_SINT: return "R64G64_SINT";
        case VK_FORMAT_R64G64_SFLOAT: return "R64G64_SFLOAT";
        case VK_FORMAT_R64G64B64_UINT: return "R64G64B64_UINT";
        case VK_FORMAT_R64G64B64_SINT: return "R64G64B64_SINT";
        case VK_FORMAT_R64G64B64_SFLOAT: return "R64G64B64_SFLOAT";
        case VK_FORMAT_R64G64B64A64_UINT: return "R64G64B64A64_UINT";
        case VK_FORMAT_R64G64B64A64_SINT: return "R64G64B64A64_SINT";
        case VK_FORMAT_R64G64B64A64_SFLOAT: return "R64G64B64A64_SFLOAT";
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return "B10G11R11_UFLOAT_PACK32";
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return "E5B9G9R9_UFLOAT_PACK32";
        case VK_FORMAT_D16_UNORM: return "D16_UNORM";
        case VK_FORMAT_X8_D24_UNORM_PACK32: return "X8_D24_UNORM_PACK32";
        case VK_FORMAT_D32_SFLOAT: return "D32_SFLOAT";
        case VK_FORMAT_S8_UINT: return "S8_UINT";
        case VK_FORMAT_D16_UNORM_S8_UINT: return "D16_UNORM_S8_UINT";
        case VK_FORMAT_D24_UNORM_S8_UINT: return "D24_UNORM_S8_UINT";
        case VK_FORMAT_D32_SFLOAT_S8_UINT: return "D32_SFLOAT_S8_UINT";
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK: return "BC1_RGB_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return "BC1_RGB_SRGB_BLOCK";
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return "BC1_RGBA_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return "BC1_RGBA_SRGB_BLOCK";
        case VK_FORMAT_BC2_UNORM_BLOCK: return "BC2_UNORM_BLOCK";
        case VK_FORMAT_BC2_SRGB_BLOCK: return "BC2_SRGB_BLOCK";
        case VK_FORMAT_BC3_UNORM_BLOCK: return "BC3_UNORM_BLOCK";
        case VK_FORMAT_BC3_SRGB_BLOCK: return "BC3_SRGB_BLOCK";
        case VK_FORMAT_BC4_UNORM_BLOCK: return "BC4_UNORM_BLOCK";
        case VK_FORMAT_BC4_SNORM_BLOCK: return "BC4_SNORM_BLOCK";
        case VK_FORMAT_BC5_UNORM_BLOCK: return "BC5_UNORM_BLOCK";
        case VK_FORMAT_BC5_SNORM_BLOCK: return "BC5_SNORM_BLOCK";
        case VK_FORMAT_BC6H_UFLOAT_BLOCK: return "BC6H_UFLOAT_BLOCK";
        case VK_FORMAT_BC6H_SFLOAT_BLOCK: return "BC6H_SFLOAT_BLOCK";
        case VK_FORMAT_BC7_UNORM_BLOCK: return "BC7_UNORM_BLOCK";
        case VK_FORMAT_BC7_SRGB_BLOCK: return "BC7_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return "ETC2_R8G8B8_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return "ETC2_R8G8B8_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return "ETC2_R8G8B8A1_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return "ETC2_R8G8B8A1_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: return "ETC2_R8G8B8A8_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return "ETC2_R8G8B8A8_SRGB_BLOCK";
        case VK_FORMAT_EAC_R11_UNORM_BLOCK: return "EAC_R11_UNORM_BLOCK";
        case VK_FORMAT_EAC_R11_SNORM_BLOCK: return "EAC_R11_SNORM_BLOCK";
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: return "EAC_R11G11_UNORM_BLOCK";
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return "EAC_R11G11_SNORM_BLOCK";
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: return "ASTC_4x4_UNORM_BLOCK";
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return "ASTC_4x4_SRGB_BLOCK";
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: return "ASTC_5x4_UNORM_BLOCK";
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return "ASTC_5x4_SRGB_BLOCK";
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: return "ASTC_5x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return "ASTC_5x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: return "ASTC_6x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return "ASTC_6x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: return "ASTC_6x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return "ASTC_6x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: return "ASTC_8x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return "ASTC_8x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: return "ASTC_8x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return "ASTC_8x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: return "ASTC_8x8_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return "ASTC_8x8_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: return "ASTC_10x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return "ASTC_10x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: return "ASTC_10x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return "ASTC_10x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: return "ASTC_10x8_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return "ASTC_10x8_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: return "ASTC_10x10_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return "ASTC_10x10_SRGB_BLOCK";
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: return "ASTC_12x10_UNORM_BLOCK";
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return "ASTC_12x10_SRGB_BLOCK";
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: return "ASTC_12x12_UNORM_BLOCK";
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return "ASTC_12x12_SRGB_BLOCK";
        default: return "UNKNOWN";
    }
}

const char* TranslateResourceAccessModeToString(ResourceAccessMode mode)
{
    switch (mode)
    {
    case ResourceAccessMode::Invalid: return "Invalid";
    case ResourceAccessMode::GPUOnly: return "GPUOnly";
    case ResourceAccessMode::Immutable: return "Immutable";
    case ResourceAccessMode::Readback: return "Readback";
    case ResourceAccessMode::Upload: return "Upload";
    case ResourceAccessMode::Volatile: return "Volatile";
    default: return "UNKNOWN";
    }
}

VkBlendFactor TranslateBlendFuncToVkBlendFactor(BlendFunc func)
{
    switch (func)
    {
    case BlendFunc::Zero: return VK_BLEND_FACTOR_ZERO;
    case BlendFunc::One: return VK_BLEND_FACTOR_ONE;
    case BlendFunc::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
    case BlendFunc::DestColor: return VK_BLEND_FACTOR_DST_COLOR;
    case BlendFunc::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
    case BlendFunc::DestAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
    case BlendFunc::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case BlendFunc::OneMinusDestColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case BlendFunc::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case BlendFunc::OneMinusDestAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    default: return VK_BLEND_FACTOR_MAX_ENUM;
    }
}

VkBlendOp TranslateBlendOpToVkBlendOp(BlendOp op)
{
    switch (op)
    {
    case BlendOp::Add: return VK_BLEND_OP_ADD;
    case BlendOp::Subtract: return VK_BLEND_OP_SUBTRACT;
    case BlendOp::RevSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case BlendOp::Min: return VK_BLEND_OP_MIN;
    case BlendOp::Max: return VK_BLEND_OP_MAX;
    default: return VK_BLEND_OP_MAX_ENUM;
    }
}

VkBufferUsageFlags TranslateBufferUsageToVkBufferUsage(BufferUsageFlag type)
{
    auto translateFlag = [&type](BufferUsageFlag flag, uint32 result) -> uint32
    {
        return ((type & flag) == flag) ? result : 0;
    };

    VkBufferUsageFlags result = 0;

    result |= translateFlag(BufferUsageFlag::IndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    result |= translateFlag(BufferUsageFlag::VertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    result |= translateFlag(BufferUsageFlag::ConstantBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    // both STRUCT_BUFFER and WRITABLE_STRUCT_BUFFER are handled with the same Vulkan usage
    // eventual "writableness" is solved in a different way
    result |= translateFlag(BufferUsageFlag::ReadonlyStruct, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    result |= translateFlag(BufferUsageFlag::WritableStruct, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    return result;
}

VkCompareOp TranslateCompareFuncToVkCompareOp(CompareFunc func)
{
    switch (func)
    {
    case CompareFunc::Never: return VK_COMPARE_OP_NEVER;
    case CompareFunc::Less: return VK_COMPARE_OP_LESS;
    case CompareFunc::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareFunc::Equal: return VK_COMPARE_OP_EQUAL;
    case CompareFunc::EqualGreater: return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case CompareFunc::Greater: return VK_COMPARE_OP_GREATER;
    case CompareFunc::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
    case CompareFunc::Pass: return VK_COMPARE_OP_ALWAYS;
    default: return VK_COMPARE_OP_MAX_ENUM;
    }
}

VkCullModeFlags TranslateCullModeToVkCullMode(CullMode mode)
{
    switch (mode)
    {
    case CullMode::Disabled: return VK_CULL_MODE_NONE;
    case CullMode::CW: return VK_CULL_MODE_FRONT_BIT; // treat CW as front
    case CullMode::CCW: return VK_CULL_MODE_BACK_BIT;
    default: return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
    }
}

VkDescriptorType TranslateDynamicResourceTypeToVkDescriptorType(ShaderResourceType type)
{
    switch (type)
    {
    case ShaderResourceType::CBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

VkFormat TranslateFormatToVkFormat(Format format)
{
    switch (format)
    {
    case Format::R8_U_Int:              return VK_FORMAT_R8_UINT;
    case Format::R8_S_Int:              return VK_FORMAT_R8_SINT;
    case Format::R8_U_Norm:             return VK_FORMAT_R8_UNORM;
    case Format::R8_S_Norm:             return VK_FORMAT_R8_SNORM;

    case Format::R16_U_Int:             return VK_FORMAT_R16_UINT;
    case Format::R16_S_Int:             return VK_FORMAT_R16_SINT;
    case Format::R16_U_Norm:            return VK_FORMAT_R16_UNORM;
    case Format::R16_S_Norm:            return VK_FORMAT_R16_SNORM;
    case Format::R16_Float:             return VK_FORMAT_R16_SFLOAT;

    case Format::R32_U_Int:             return VK_FORMAT_R32_UINT;
    case Format::R32_S_Int:             return VK_FORMAT_R32_SINT;
    case Format::R32_Float:             return VK_FORMAT_R32_SFLOAT;

    case Format::R8G8_U_Int:            return VK_FORMAT_R8G8_UINT;
    case Format::R8G8_S_Int:            return VK_FORMAT_R8G8_SINT;
    case Format::R8G8_U_Norm:           return VK_FORMAT_R8G8_UNORM;
    case Format::R8G8_S_Norm:           return VK_FORMAT_R8G8_SNORM;

    case Format::R16G16_U_Int:          return VK_FORMAT_R16G16_UINT;
    case Format::R16G16_S_Int:          return VK_FORMAT_R16G16_SINT;
    case Format::R16G16_U_Norm:         return VK_FORMAT_R16G16_UNORM;
    case Format::R16G16_S_Norm:         return VK_FORMAT_R16G16_SNORM;
    case Format::R16G16_Float:          return VK_FORMAT_R16G16_SFLOAT;

    case Format::R32G32_U_Int:          return VK_FORMAT_R32G32_UINT;
    case Format::R32G32_S_Int:          return VK_FORMAT_R32G32_SINT;
    case Format::R32G32_Float:          return VK_FORMAT_R32G32_SFLOAT;

    case Format::R32G32B32_U_Int:       return VK_FORMAT_R32G32B32_UINT;
    case Format::R32G32B32_S_Int:       return VK_FORMAT_R32G32B32_SINT;
    case Format::R32G32B32_Float:       return VK_FORMAT_R32G32B32_SFLOAT;

    case Format::R8G8B8A8_U_Int:        return VK_FORMAT_R8G8B8A8_UINT;
    case Format::R8G8B8A8_S_Int:        return VK_FORMAT_R8G8B8A8_SINT;
    case Format::R8G8B8A8_U_Norm:       return VK_FORMAT_R8G8B8A8_UNORM;
    case Format::R8G8B8A8_U_Norm_sRGB:  return VK_FORMAT_R8G8B8A8_SRGB;
    case Format::R8G8B8A8_S_Norm:       return VK_FORMAT_R8G8B8A8_SNORM;
    case Format::B8G8R8A8_U_Norm:       return VK_FORMAT_B8G8R8A8_UNORM;
    case Format::B8G8R8A8_U_Norm_sRGB:  return VK_FORMAT_B8G8R8A8_SRGB;

    case Format::R16G16B16A16_U_Int:    return VK_FORMAT_R16G16B16A16_UINT;
    case Format::R16G16B16A16_S_Int:    return VK_FORMAT_R16G16B16A16_SINT;
    case Format::R16G16B16A16_U_Norm:   return VK_FORMAT_R16G16B16A16_UNORM;
    case Format::R16G16B16A16_S_Norm:   return VK_FORMAT_R16G16B16A16_SNORM;
    case Format::R16G16B16A16_Float:    return VK_FORMAT_R16G16B16A16_SFLOAT;

    case Format::R32G32B32A32_U_Int:    return VK_FORMAT_R32G32B32A32_UINT;
    case Format::R32G32B32A32_S_Int:    return VK_FORMAT_R32G32B32A32_SINT;
    case Format::R32G32B32A32_Float:    return VK_FORMAT_R32G32B32A32_SFLOAT;

    case Format::B5G6R5_U_norm:         return VK_FORMAT_B5G6R5_UNORM_PACK16;
    case Format::B5G5R5A1_U_Norm:       return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
    case Format::B4G4R4A4_U_Norm:       return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
    case Format::R11G11B10_Float:       return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case Format::R10G10B10A2_U_Norm:    return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case Format::R10G10B10A2_U_Int:     return VK_FORMAT_A2B10G10R10_UINT_PACK32;
    case Format::R9G9B9E5_Float:        return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

    case Format::BC1_U_Norm:            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case Format::BC1_U_Norm_sRGB:       return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case Format::BC2_U_Norm:            return VK_FORMAT_BC2_UNORM_BLOCK;
    case Format::BC2_U_Norm_sRGB:       return VK_FORMAT_BC2_SRGB_BLOCK;
    case Format::BC3_U_Norm:            return VK_FORMAT_BC3_UNORM_BLOCK;
    case Format::BC3_U_Norm_sRGB:       return VK_FORMAT_BC3_SRGB_BLOCK;
    case Format::BC4_U_Norm:            return VK_FORMAT_BC4_UNORM_BLOCK;
    case Format::BC4_S_Norm:            return VK_FORMAT_BC4_SNORM_BLOCK;
    case Format::BC5_U_Norm:            return VK_FORMAT_BC5_UNORM_BLOCK;
    case Format::BC5_S_Norm:            return VK_FORMAT_BC5_SNORM_BLOCK;
    case Format::BC6H_U_Float:          return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case Format::BC6H_S_Float:          return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case Format::BC7_U_Norm:            return VK_FORMAT_BC7_UNORM_BLOCK;
    case Format::BC7_U_Norm_sRGB:       return VK_FORMAT_BC7_SRGB_BLOCK;

    case Format::Depth16:               return VK_FORMAT_D16_UNORM;
    case Format::Depth24_Stencil8:      return VK_FORMAT_D24_UNORM_S8_UINT;
    case Format::Depth32:               return VK_FORMAT_D32_SFLOAT;
    case Format::Depth32_Stencil8:      return VK_FORMAT_D32_SFLOAT_S8_UINT;
    }

    static_assert(68 == (uint32)Format::Max, "Format list changed, update the switch above");

    return VK_FORMAT_UNDEFINED;
}

VkPolygonMode TranslateFillModeToVkPolygonMode(FillMode mode)
{
    switch (mode)
    {
    case FillMode::Solid: return VK_POLYGON_MODE_FILL;
    case FillMode::Wireframe: return VK_POLYGON_MODE_LINE;
    default: return VK_POLYGON_MODE_MAX_ENUM;
    }
}

VkIndexType TranslateIndexBufferFormatToVkIndexType(IndexBufferFormat format)
{
    switch (format)
    {
    case IndexBufferFormat::Uint16: return VK_INDEX_TYPE_UINT16;
    case IndexBufferFormat::Uint32: return VK_INDEX_TYPE_UINT32;
    default: return VK_INDEX_TYPE_MAX_ENUM;
    }
}

VkFilter TranslateMagFilterToVkFilter(TextureMagFilter filter)
{
    switch (filter)
    {
    case TextureMagFilter::Linear: return VK_FILTER_LINEAR;
    case TextureMagFilter::Nearest: return VK_FILTER_NEAREST;
    default: return VK_FILTER_MAX_ENUM;
    }
}

VkFilter TranslateMinFilterToVkFilter(TextureMinFilter filter)
{
    switch (filter)
    {
    case TextureMinFilter::LinearMipmapLinear:
    case TextureMinFilter::LinearMipmapNearest:
        return VK_FILTER_LINEAR;
    case TextureMinFilter::NearestMipmapLinear:
    case TextureMinFilter::NearestMipmapNearest:
        return VK_FILTER_NEAREST;
    default: return VK_FILTER_MAX_ENUM;
    }
}

VkSamplerMipmapMode TranslateMinFilterToVkSamplerMipmapMode(TextureMinFilter filter)
{
    switch (filter)
    {
    case TextureMinFilter::LinearMipmapLinear:
    case TextureMinFilter::NearestMipmapLinear:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    case TextureMinFilter::LinearMipmapNearest:
    case TextureMinFilter::NearestMipmapNearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    default: return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
    }
}

VkPrimitiveTopology TranslatePrimitiveTypeToVkTopology(PrimitiveType type)
{
    switch (type)
    {
    case PrimitiveType::Points: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case PrimitiveType::Lines: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case PrimitiveType::LinesStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case PrimitiveType::Triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case PrimitiveType::TrianglesStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case PrimitiveType::Patch: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    default: return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    }
}

VkSampleCountFlagBits TranslateSamplesNumToVkSampleCount(uint32 sampleNum)
{
    switch (sampleNum)
    {
    case 1: return VK_SAMPLE_COUNT_1_BIT;
    case 2: return VK_SAMPLE_COUNT_2_BIT;
    case 4: return VK_SAMPLE_COUNT_4_BIT;
    case 8: return VK_SAMPLE_COUNT_8_BIT;
    case 16: return VK_SAMPLE_COUNT_16_BIT;
    case 32: return VK_SAMPLE_COUNT_32_BIT;
    case 64: return VK_SAMPLE_COUNT_64_BIT;
    default: return static_cast<VkSampleCountFlagBits>(0);
    }
}

VkDescriptorType TranslateShaderResourceTypeToVkDescriptorType(ShaderResourceType type)
{
    switch (type)
    {
    case ShaderResourceType::CBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case ShaderResourceType::Texture: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case ShaderResourceType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
    case ShaderResourceType::WritableTexture: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

VkShaderStageFlagBits TranslateShaderTypeToVkShaderStage(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
    case ShaderType::Hull: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case ShaderType::Domain: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case ShaderType::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
    case ShaderType::Pixel: return VK_SHADER_STAGE_FRAGMENT_BIT;
    case ShaderType::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
    case ShaderType::All: return VK_SHADER_STAGE_ALL_GRAPHICS;
    default: return static_cast<VkShaderStageFlagBits>(0);
    }
}

VkStencilOp TranslateStencilOpToVkStencilOp(StencilOp op)
{
    switch (op)
    {
    case StencilOp::Keep: return VK_STENCIL_OP_KEEP;
    case StencilOp::Zero: return VK_STENCIL_OP_ZERO;
    case StencilOp::Replace: return VK_STENCIL_OP_REPLACE;
    case StencilOp::Increment: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case StencilOp::IncrementWrap: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case StencilOp::Decrement: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case StencilOp::DecrementWrap: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    case StencilOp::Invert: return VK_STENCIL_OP_INVERT;
    default: return VK_STENCIL_OP_MAX_ENUM;
    }
}

VkSamplerAddressMode TranslateWrapModeToVkSamplerAddressMode(TextureWrapMode mode)
{
    switch (mode)
    {
    case TextureWrapMode::Border: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case TextureWrapMode::Clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case TextureWrapMode::Mirror: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case TextureWrapMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    default: return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
    }
}

} // namespace Renderer
} // namespace NFE

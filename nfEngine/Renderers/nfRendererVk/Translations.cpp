/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
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

VkBufferUsageFlags TranslateBufferTypeToVkBufferUsage(BufferType type)
{
    switch (type)
    {
    case BufferType::Constant: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    case BufferType::Index: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    case BufferType::Vertex: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    default: return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
    }
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

VkFormat TranslateElementFormatToVkFormat(ElementFormat format, int size)
{
    switch (format)
    {
    case ElementFormat::Float_32:
        switch (size)
        {
        case 1:
            return VK_FORMAT_R32_SFLOAT;
        case 2:
            return VK_FORMAT_R32G32_SFLOAT;
        case 3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case 4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        };
    case ElementFormat::Int_32:
        switch (size)
        {
        case 1:
            return VK_FORMAT_R32_SINT;
        case 2:
            return VK_FORMAT_R32G32_SINT;
        case 3:
            return VK_FORMAT_R32G32B32_SINT;
        case 4:
            return VK_FORMAT_R32G32B32A32_SINT;
        };
    case ElementFormat::Uint_32:
        switch (size)
        {
        case 1:
            return VK_FORMAT_R32_UINT;
        case 2:
            return VK_FORMAT_R32G32_UINT;
        case 3:
            return VK_FORMAT_R32G32B32_UINT;
        case 4:
            return VK_FORMAT_R32G32B32A32_UINT;
        };
    case ElementFormat::Float_16:
        switch (size)
        {
        case 1:
            return VK_FORMAT_R16_SFLOAT;
        case 2:
            return VK_FORMAT_R16G16_SFLOAT;
        case 4:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        };
    case ElementFormat::Int_16:
        switch (size)
        {
        case 1:
            return VK_FORMAT_R16_SINT;
        case 2:
            return VK_FORMAT_R16G16_SINT;
        case 4:
            return VK_FORMAT_R16G16B16A16_SINT;
        };
    case ElementFormat::Uint_16:
        switch (size)
        {
        case 1:
            return VK_FORMAT_R16_UINT;
        case 2:
            return VK_FORMAT_R16G16_UINT;
        case 4:
            return VK_FORMAT_R16G16B16A16_UINT;
        };
    case ElementFormat::Int_16_norm:
        switch (size)
        {
        case 1:
            return VK_FORMAT_R16_SNORM;
        case 2:
            return VK_FORMAT_R16G16_SNORM;
        case 4:
            return VK_FORMAT_R16G16B16A16_SNORM;
        };
    case ElementFormat::Uint_16_norm:
        switch (size)
        {
        case 1:
            return VK_FORMAT_R16_UNORM;
        case 2:
            return VK_FORMAT_R16G16_UNORM;
        case 4:
            return VK_FORMAT_R16G16B16A16_UNORM;
        };
    case ElementFormat::Int_8:
        switch (size)
        {
        case 1:
            return VK_FORMAT_R8_SINT;
        case 2:
            return VK_FORMAT_R8G8_SINT;
        case 4:
            return VK_FORMAT_R8G8B8A8_SINT;
        };
    case ElementFormat::Uint_8:
        switch (size)
        {
        case 1:
            return VK_FORMAT_R8_UINT;
        case 2:
            return VK_FORMAT_R8G8_UINT;
        case 4:
            return VK_FORMAT_R8G8B8A8_UINT;
        };
    case ElementFormat::Int_8_norm:
        switch (size)
        {
        case 1:
            return VK_FORMAT_R8_SNORM;
        case 2:
            return VK_FORMAT_R8G8_SNORM;
        case 4:
            return VK_FORMAT_R8G8B8A8_SNORM;
        };
    case ElementFormat::Uint_8_norm:
        switch (size)
        {
        case 1:
            return VK_FORMAT_R8_UNORM;
        case 2:
            return VK_FORMAT_R8G8_UNORM;
        case 4:
            return VK_FORMAT_R8G8B8A8_UNORM;
        };
    case ElementFormat::BC1:
        return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case ElementFormat::BC2:
        return VK_FORMAT_BC2_UNORM_BLOCK;
    case ElementFormat::BC3:
        return VK_FORMAT_BC3_UNORM_BLOCK;
    case ElementFormat::BC4:
        return VK_FORMAT_BC4_UNORM_BLOCK;
    case ElementFormat::BC4_signed:
        return VK_FORMAT_BC4_SNORM_BLOCK;
    case ElementFormat::BC5:
        return VK_FORMAT_BC5_UNORM_BLOCK;
    case ElementFormat::BC5_signed:
        return VK_FORMAT_BC5_SNORM_BLOCK;
    case ElementFormat::BC6H:
        return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case ElementFormat::BC6H_signed:
        return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case ElementFormat::BC7:
        return VK_FORMAT_BC7_UNORM_BLOCK;
    default:
        return VK_FORMAT_UNDEFINED;
    };
}

uint32 TranslateElementFormatToByteSize(ElementFormat format)
{
    switch (format)
    {
    case ElementFormat::Float_32:
    case ElementFormat::Int_32:
    case ElementFormat::Uint_32:
        return 4;
    case ElementFormat::Float_16:
    case ElementFormat::Int_16:
    case ElementFormat::Uint_16:
    case ElementFormat::Int_16_norm:
    case ElementFormat::Uint_16_norm:
        return 2;
    case ElementFormat::Int_8:
    case ElementFormat::Uint_8:
    case ElementFormat::Int_8_norm:
    case ElementFormat::Uint_8_norm:
        return 1;
    default:
        return 0;
    };
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

} // namespace Renderer
} // namespace NFE

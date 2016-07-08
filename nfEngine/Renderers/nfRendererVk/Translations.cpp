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
    switch(type)
    {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "OTHER";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "INTEGRATED_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "DISCRETE_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "VIRTUAL_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
    default: return "UNKNOWN";
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

const char* TranslateVkResultToString(VkResult result)
{
    switch(result)
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

} // namespace Renderer
} // namespace NFE

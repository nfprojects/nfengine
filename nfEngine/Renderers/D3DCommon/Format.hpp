/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of common for Direct3D format translations.
 */

#pragma once

namespace NFE {
namespace Renderer {

DXGI_FORMAT TranslateElementFormat(ElementFormat format)
{
    switch (format)
    {
    case ElementFormat::R8_U_Int:               return DXGI_FORMAT_R8_UINT;
    case ElementFormat::R8_S_Int:               return DXGI_FORMAT_R8_SINT;
    case ElementFormat::R8_U_Norm:              return DXGI_FORMAT_R8_UNORM;
    case ElementFormat::R8_S_Norm:              return DXGI_FORMAT_R8_SNORM;

    case ElementFormat::R16_U_Int:              return DXGI_FORMAT_R16_UINT;
    case ElementFormat::R16_S_Int:              return DXGI_FORMAT_R16_SINT;
    case ElementFormat::R16_U_Norm:             return DXGI_FORMAT_R16_UNORM;
    case ElementFormat::R16_S_Norm:             return DXGI_FORMAT_R16_SNORM;
    case ElementFormat::R16_Float:              return DXGI_FORMAT_R16_FLOAT;

    case ElementFormat::R32_U_Int:              return DXGI_FORMAT_R32_UINT;
    case ElementFormat::R32_S_Int:              return DXGI_FORMAT_R32_SINT;
    case ElementFormat::R32_Float:              return DXGI_FORMAT_R32_FLOAT;

    case ElementFormat::R8G8_U_Int:             return DXGI_FORMAT_R8G8_UINT;
    case ElementFormat::R8G8_S_Int:             return DXGI_FORMAT_R8G8_SINT;
    case ElementFormat::R8G8_U_Norm:            return DXGI_FORMAT_R8G8_UNORM;
    case ElementFormat::R8G8_S_Norm:            return DXGI_FORMAT_R8G8_SNORM;

    case ElementFormat::R16G16_U_Int:           return DXGI_FORMAT_R16G16_UINT;
    case ElementFormat::R16G16_S_Int:           return DXGI_FORMAT_R16G16_SINT;
    case ElementFormat::R16G16_U_Norm:          return DXGI_FORMAT_R16G16_UNORM;
    case ElementFormat::R16G16_S_Norm:          return DXGI_FORMAT_R16G16_SNORM;
    case ElementFormat::R16G16_Float:           return DXGI_FORMAT_R16G16_FLOAT;

    case ElementFormat::R32G32_U_Int:           return DXGI_FORMAT_R32G32_UINT;
    case ElementFormat::R32G32_S_Int:           return DXGI_FORMAT_R32G32_SINT;
    case ElementFormat::R32G32_Float:           return DXGI_FORMAT_R32G32_FLOAT;

    case ElementFormat::R32G32B32_U_Int:        return DXGI_FORMAT_R32G32B32_UINT;
    case ElementFormat::R32G32B32_S_Int:        return DXGI_FORMAT_R32G32B32_SINT;
    case ElementFormat::R32G32B32_Float:        return DXGI_FORMAT_R32G32B32_FLOAT;

    case ElementFormat::R8G8B8A8_U_Int:         return DXGI_FORMAT_R8G8B8A8_UINT;
    case ElementFormat::R8G8B8A8_S_Int:         return DXGI_FORMAT_R8G8B8A8_SINT;
    case ElementFormat::R8G8B8A8_U_Norm:        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case ElementFormat::R8G8B8A8_U_Norm_sRGB:   return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case ElementFormat::R8G8B8A8_S_Norm:        return DXGI_FORMAT_R8G8B8A8_SNORM;
    case ElementFormat::B8G8R8A8_U_Norm:        return DXGI_FORMAT_R32G32_UINT;
    case ElementFormat::B8G8R8A8_U_Norm_sRGB:   return DXGI_FORMAT_R32G32_UINT;

    case ElementFormat::R16G16B16A16_U_Int:     return DXGI_FORMAT_R16G16B16A16_UINT;
    case ElementFormat::R16G16B16A16_S_Int:     return DXGI_FORMAT_R16G16B16A16_SINT;
    case ElementFormat::R16G16B16A16_U_Norm:    return DXGI_FORMAT_R16G16B16A16_UNORM;
    case ElementFormat::R16G16B16A16_S_Norm:    return DXGI_FORMAT_R16G16B16A16_SNORM;
    case ElementFormat::R16G16B16A16_Float:     return DXGI_FORMAT_R16G16B16A16_FLOAT;

    case ElementFormat::R32G32B32A32_U_Int:     return DXGI_FORMAT_R32G32B32A32_UINT;
    case ElementFormat::R32G32B32A32_S_Int:     return DXGI_FORMAT_R32G32B32A32_SINT;
    case ElementFormat::R32G32B32A32_Float:     return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case ElementFormat::B5G6R5_U_norm:          return DXGI_FORMAT_B5G6R5_UNORM;
    case ElementFormat::B5G5R5A1_U_Norm:        return DXGI_FORMAT_B5G5R5A1_UNORM;
    case ElementFormat::B4G4R4A4_U_Norm:        return DXGI_FORMAT_B4G4R4A4_UNORM;
    case ElementFormat::R11G11B10_Float:        return DXGI_FORMAT_R11G11B10_FLOAT;
    case ElementFormat::R10G10B10A2_U_Norm:     return DXGI_FORMAT_R10G10B10A2_UNORM;
    case ElementFormat::R10G10B10A2_U_Int:      return DXGI_FORMAT_R10G10B10A2_UINT;
    case ElementFormat::R9G9B9E5_Float:         return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

    case ElementFormat::BC1_U_Norm:             return DXGI_FORMAT_BC1_UNORM;
    case ElementFormat::BC1_U_Norm_sRGB:        return DXGI_FORMAT_BC1_UNORM_SRGB;
    case ElementFormat::BC2_U_Norm:             return DXGI_FORMAT_BC2_UNORM;
    case ElementFormat::BC2_U_Norm_sRGB:        return DXGI_FORMAT_BC2_UNORM_SRGB;
    case ElementFormat::BC3_U_Norm:             return DXGI_FORMAT_BC3_UNORM;
    case ElementFormat::BC3_U_Norm_sRGB:        return DXGI_FORMAT_BC3_UNORM_SRGB;
    case ElementFormat::BC4_U_Norm:             return DXGI_FORMAT_BC4_UNORM;
    case ElementFormat::BC4_S_Norm:             return DXGI_FORMAT_BC4_SNORM;
    case ElementFormat::BC5_U_Norm:             return DXGI_FORMAT_BC5_UNORM;
    case ElementFormat::BC5_S_Norm:             return DXGI_FORMAT_BC5_SNORM;
    case ElementFormat::BC6H_U_Float:           return DXGI_FORMAT_BC6H_UF16;
    case ElementFormat::BC6H_S_Float:           return DXGI_FORMAT_BC6H_SF16;
    case ElementFormat::BC7_U_Norm:             return DXGI_FORMAT_BC7_UNORM;
    case ElementFormat::BC7_U_Norm_sRGB:        return DXGI_FORMAT_BC7_UNORM_SRGB;
    }

    return DXGI_FORMAT_UNKNOWN;
}

} // namespace Renderer
} // namespace NFE

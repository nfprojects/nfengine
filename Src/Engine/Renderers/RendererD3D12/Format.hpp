/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of common for Direct3D format translations.
 */

#pragma once

namespace NFE {
namespace Renderer {

NFE_INLINE DXGI_FORMAT TranslateFormat(Format format)
{
    switch (format)
    {
    case Format::R8_U_Int:              return DXGI_FORMAT_R8_UINT;
    case Format::R8_S_Int:              return DXGI_FORMAT_R8_SINT;
    case Format::R8_U_Norm:             return DXGI_FORMAT_R8_UNORM;
    case Format::R8_S_Norm:             return DXGI_FORMAT_R8_SNORM;

    case Format::R16_U_Int:             return DXGI_FORMAT_R16_UINT;
    case Format::R16_S_Int:             return DXGI_FORMAT_R16_SINT;
    case Format::R16_U_Norm:            return DXGI_FORMAT_R16_UNORM;
    case Format::R16_S_Norm:            return DXGI_FORMAT_R16_SNORM;
    case Format::R16_Float:             return DXGI_FORMAT_R16_FLOAT;

    case Format::R32_U_Int:             return DXGI_FORMAT_R32_UINT;
    case Format::R32_S_Int:             return DXGI_FORMAT_R32_SINT;
    case Format::R32_Float:             return DXGI_FORMAT_R32_FLOAT;

    case Format::R8G8_U_Int:            return DXGI_FORMAT_R8G8_UINT;
    case Format::R8G8_S_Int:            return DXGI_FORMAT_R8G8_SINT;
    case Format::R8G8_U_Norm:           return DXGI_FORMAT_R8G8_UNORM;
    case Format::R8G8_S_Norm:           return DXGI_FORMAT_R8G8_SNORM;

    case Format::R16G16_U_Int:          return DXGI_FORMAT_R16G16_UINT;
    case Format::R16G16_S_Int:          return DXGI_FORMAT_R16G16_SINT;
    case Format::R16G16_U_Norm:         return DXGI_FORMAT_R16G16_UNORM;
    case Format::R16G16_S_Norm:         return DXGI_FORMAT_R16G16_SNORM;
    case Format::R16G16_Float:          return DXGI_FORMAT_R16G16_FLOAT;

    case Format::R32G32_U_Int:          return DXGI_FORMAT_R32G32_UINT;
    case Format::R32G32_S_Int:          return DXGI_FORMAT_R32G32_SINT;
    case Format::R32G32_Float:          return DXGI_FORMAT_R32G32_FLOAT;

    case Format::R32G32B32_U_Int:       return DXGI_FORMAT_R32G32B32_UINT;
    case Format::R32G32B32_S_Int:       return DXGI_FORMAT_R32G32B32_SINT;
    case Format::R32G32B32_Float:       return DXGI_FORMAT_R32G32B32_FLOAT;

    case Format::R8G8B8A8_U_Int:        return DXGI_FORMAT_R8G8B8A8_UINT;
    case Format::R8G8B8A8_S_Int:        return DXGI_FORMAT_R8G8B8A8_SINT;
    case Format::R8G8B8A8_U_Norm:       return DXGI_FORMAT_R8G8B8A8_UNORM;
    case Format::R8G8B8A8_U_Norm_sRGB:  return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case Format::R8G8B8A8_S_Norm:       return DXGI_FORMAT_R8G8B8A8_SNORM;
    case Format::B8G8R8A8_U_Norm:       return DXGI_FORMAT_B8G8R8A8_UNORM;
    case Format::B8G8R8A8_U_Norm_sRGB:  return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    case Format::R16G16B16A16_U_Int:    return DXGI_FORMAT_R16G16B16A16_UINT;
    case Format::R16G16B16A16_S_Int:    return DXGI_FORMAT_R16G16B16A16_SINT;
    case Format::R16G16B16A16_U_Norm:   return DXGI_FORMAT_R16G16B16A16_UNORM;
    case Format::R16G16B16A16_S_Norm:   return DXGI_FORMAT_R16G16B16A16_SNORM;
    case Format::R16G16B16A16_Float:    return DXGI_FORMAT_R16G16B16A16_FLOAT;

    case Format::R32G32B32A32_U_Int:    return DXGI_FORMAT_R32G32B32A32_UINT;
    case Format::R32G32B32A32_S_Int:    return DXGI_FORMAT_R32G32B32A32_SINT;
    case Format::R32G32B32A32_Float:    return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case Format::B5G6R5_U_norm:         return DXGI_FORMAT_B5G6R5_UNORM;
    case Format::B5G5R5A1_U_Norm:       return DXGI_FORMAT_B5G5R5A1_UNORM;
    case Format::B4G4R4A4_U_Norm:       return DXGI_FORMAT_B4G4R4A4_UNORM;
    case Format::R11G11B10_Float:       return DXGI_FORMAT_R11G11B10_FLOAT;
    case Format::R10G10B10A2_U_Norm:    return DXGI_FORMAT_R10G10B10A2_UNORM;
    case Format::R10G10B10A2_U_Int:     return DXGI_FORMAT_R10G10B10A2_UINT;
    case Format::R9G9B9E5_Float:        return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

    case Format::BC1_U_Norm:            return DXGI_FORMAT_BC1_UNORM;
    case Format::BC1_U_Norm_sRGB:       return DXGI_FORMAT_BC1_UNORM_SRGB;
    case Format::BC2_U_Norm:            return DXGI_FORMAT_BC2_UNORM;
    case Format::BC2_U_Norm_sRGB:       return DXGI_FORMAT_BC2_UNORM_SRGB;
    case Format::BC3_U_Norm:            return DXGI_FORMAT_BC3_UNORM;
    case Format::BC3_U_Norm_sRGB:       return DXGI_FORMAT_BC3_UNORM_SRGB;
    case Format::BC4_U_Norm:            return DXGI_FORMAT_BC4_UNORM;
    case Format::BC4_S_Norm:            return DXGI_FORMAT_BC4_SNORM;
    case Format::BC5_U_Norm:            return DXGI_FORMAT_BC5_UNORM;
    case Format::BC5_S_Norm:            return DXGI_FORMAT_BC5_SNORM;
    case Format::BC6H_U_Float:          return DXGI_FORMAT_BC6H_UF16;
    case Format::BC6H_S_Float:          return DXGI_FORMAT_BC6H_SF16;
    case Format::BC7_U_Norm:            return DXGI_FORMAT_BC7_UNORM;
    case Format::BC7_U_Norm_sRGB:       return DXGI_FORMAT_BC7_UNORM_SRGB;

    case Format::Depth16:               return DXGI_FORMAT_D16_UNORM;
    case Format::Depth24_Stencil8:      return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case Format::Depth32:               return DXGI_FORMAT_D32_FLOAT;
    case Format::Depth32_Stencil8:      return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    }

    static_assert(68 == (uint32)Format::Max, "Format list changed, update the switch above");

    return DXGI_FORMAT_UNKNOWN;
}

} // namespace Renderer
} // namespace NFE

/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Enums and functions used by Image class
 */

#pragma once
#include "../nfCommon.hpp"

namespace NFE {
namespace Common {

enum class MipmapFilter
{
    Box,
    GammaCorrectedLinear,

    Unknown,
};

enum class ImageFormat
{
    A_UByte,     // 8 bit
    R_UByte,     // 8 bit
    RGB_UByte,   // 24 bit
    RGBA_UByte,  // 32 bit

    R_Float,     // 32 bit
    RGBA_Float,  // 128 bit

    BC1,         // block coding 1,  aka. DXT1 - 4 bits per pixel
    BC2,         // block coding 2,  aka. DXT3 - 8 bits per pixel
    BC3,         // block coding 3,  aka. DXT5 - 8 bits per pixel
    BC4,         // block coding 4,  aka. ATI1 - 4 bits per pixel
    BC5,         // block coding 5,  aka. ATI2 - 8 bits per pixel
    BC6H,        // block coding 6h, aka. BPTC - 8 bits per pixel
    BC7,         // block coding 7,  aka. BPTC - 8 bits per pixel

    Unknown,
};

/**
 * Convert MipmapFilter to string
 * @param filter Filter format
 */
NFCOMMON_API const char* FilterToStr(MipmapFilter filter);

/**
 * Convert ImageFormat to string
 * @param format Source data format
 */
NFCOMMON_API const char* FormatToStr(ImageFormat format);

/**
 * Retrieve number of bits per pixel for an image format
 * @param format Source data format
 */
NFCOMMON_API size_t BitsPerPixel(ImageFormat format);

/**
 * Checks whether given format is a supported BC format
 * @param format Source data format
 */
NFCOMMON_API bool IsSupportedBC(ImageFormat format);

} // namespace Common
} // namespace NFE
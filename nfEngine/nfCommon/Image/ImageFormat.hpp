/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Enums and functions used by Image class
 */

#pragma once

namespace NFE {
namespace Common {

enum class MipmapFilter
{
    Unknown,

    Box,
    GammaCorrectedLinear,
};

enum class ImageFormat
{
    Unknown,

    A_UByte,     // 8 bit
    R_UByte,     // 8 bit
    RGB_UByte,   // 24 bit
    RGBA_UByte,  // 32 bit

    R_Float,     // 32 bit
    RGBA_Float,  // 128 bit

    BC1,         // block coding 1, aka. DXT1 - 4 bits per pixel
    BC2,         // block coding 2, aka. DXT3 - 8 bits per pixel
    BC3,         // block coding 3, aka. DXT5 - 8 bits per pixel
    BC4,         // block coding 4 - 4 bits per pixel
    BC5,         // block coding 5 - 8 bits per pixel
};

/**
 * Convert MipmapFilter to string
 * @param filter Filter format
 */
const char* FilterToStr(MipmapFilter filter);

/**
 * Convert ImageFormat to string
 * @param format Source data format
 */
const char* FormatToStr(ImageFormat format);

/**
 * Retrieve number of bits per pixel for an image format
 * @param format Source data format
 */
size_t BitsPerPixel(ImageFormat format);

} // namespace Common
} // namespace NFE
/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Image class declaration.
 */

#pragma once

#include "Image.hpp"
#include "Math/Math.hpp"

namespace NFE {
namespace Common {
    
using namespace Math;

/**
 * Image formats
 */
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
 * Get texel from raw data buffer, knowing pixel format and image width
 */
Vector GetTexel(const void* pData, uint32 x, uint32 y, uint32 width, ImageFormat fmt);

/**
 * Set texel of raw data buffer, knowing pixel format and image width
 */
void SetTexel(const Vector& v, void* pData, uint32 x, uint32 y, uint32 width, ImageFormat fmt);

} // Common
} // NFE
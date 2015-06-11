/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Enums and functions used by Image class
 */

#include "../PCH.hpp"
#include "ImageFormat.hpp"

const char* FilterToStr(MipmapFilter filter)
{
    switch (filter)
    {
    case MipmapFilter::Box:
        return "BoxFilter";
    case MipmapFilter::GammaCorrectedLinear:
        return "GammaCorrectedLinearFilter";
    }

    return "Unknown";
}

const char* FormatToStr(ImageFormat format)
{
    switch (format)
    {
    case ImageFormat::A_UByte:
        return "A_uchar";
    case ImageFormat::R_UByte:
        return "R_uchar";
    case ImageFormat::RGB_UByte:
        return "RGB_uchar";
    case ImageFormat::RGBA_UByte:
        return "RGBA_uchar";

    case ImageFormat::R_Float:
        return "R_Float";
    case ImageFormat::RGBA_Float:
        return "RGBA_Float";

    case ImageFormat::BC1:
        return "BC1 (DXT1)";
    case ImageFormat::BC2:
        return "BC2 (DXT3)";
    case ImageFormat::BC3:
        return "BC3 (DXT5)";
    case ImageFormat::BC4:
        return "BC4";
    case ImageFormat::BC5:
        return "BC5";
    };

    return "Unknown";
}

size_t BitsPerPixel(ImageFormat format)
{
    switch (format)
    {
    case ImageFormat::A_UByte:
    case ImageFormat::R_UByte:
        return 8;

    case ImageFormat::RGB_UByte:
        return 3 * 8;
    case ImageFormat::RGBA_UByte:
        return 4 * 8;

    case ImageFormat::R_Float:
        return sizeof(float) * 8;
    case ImageFormat::RGBA_Float:
        return 4 * sizeof(float) * 8;

    case ImageFormat::BC1:
    case ImageFormat::BC4:
        return 4;

    case ImageFormat::BC2:
    case ImageFormat::BC3:
    case ImageFormat::BC5:
        return 8;
    };

    return 0;
}
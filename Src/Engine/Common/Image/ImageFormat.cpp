/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Enums and functions used by Image class
 */

#include "PCH.hpp"
#include "ImageFormat.hpp"


namespace NFE {
namespace Common {

const char* FilterToStr(MipmapFilter filter)
{
    switch (filter)
    {
    case MipmapFilter::Box:
        return "BoxFilter";
    case MipmapFilter::GammaCorrectedLinear:
        return "GammaCorrectedLinearFilter";

    default:
        return "Unknown";
    }
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
        return "BC4 (ATI1)";
    case ImageFormat::BC5:
        return "BC5 (ATI2)";
    case ImageFormat::BC6H:
        return "BC6H (BPTC)";
    case ImageFormat::BC7:
        return "BC7 (BPTC)";

    default:
        return "Unknown";
    }
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
    case ImageFormat::BC6H:
    case ImageFormat::BC7:
        return 8;

    default:
        return 0;
    }
}

bool IsSupportedBC(ImageFormat format)
{
    if (format == ImageFormat::BC1 || format == ImageFormat::BC2 ||
        format == ImageFormat::BC3 || format == ImageFormat::BC4 ||
        format == ImageFormat::BC5)
        return true;
    else
        return false;
}

} // namespace Common
} // namespace NFE

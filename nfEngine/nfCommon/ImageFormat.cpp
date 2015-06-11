/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Image function definitions.
 */
 
#include "PCH.hpp"
#include "ImageFormat.hpp"

namespace NFE {
namespace Common {
    
using namespace Math;

const char* Image::FormatToStr(ImageFormat format)
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

size_t Image::BitsPerPixel(ImageFormat format)
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

Vector GetTexel(const void* pData, uint32 x, uint32 y, uint32 width, ImageFormat fmt)
{
    switch (fmt)
    {
        case ImageFormat::A_UByte:
        {
            const uchar* pSrc = (const uchar*)pData;
            uchar a = pSrc[y * width + x];
            return Vector(255.0f, 255.0f, 255.0f, (float)a) * g_Byte2Float;
        }

        case ImageFormat::R_UByte:
        {
            const uchar* pSrc = (const uchar*)pData;
            uchar r = pSrc[y * width + x];
            return Vector((float)r, 0.0f, 0.0f, 255.0f) * g_Byte2Float;
        }

        case ImageFormat::RGB_UByte:
        {
            const uchar* pSrc = (const uchar*)pData;
            uchar r = pSrc[3 * (y * width + x)];
            uchar g = pSrc[3 * (y * width + x) + 1];
            uchar b = pSrc[3 * (y * width + x) + 2];
            return Vector((float)r, (float)g, (float)b, 255.0f) * g_Byte2Float;
        }

        case ImageFormat::RGBA_UByte:
        {
            const uchar* pSrc = (const uchar*)pData;
            uchar r = pSrc[4 * (y * width + x)];
            uchar g = pSrc[4 * (y * width + x) + 1];
            uchar b = pSrc[4 * (y * width + x) + 2];
            uchar a = pSrc[4 * (y * width + x) + 3];
            return Vector((float)r, (float)g, (float)b, float(a)) * g_Byte2Float;
        }

        case ImageFormat::R_Float:
        {
            const float* pSrc = (const float*)pData;
            float r = pSrc[y * width + x];
            return Vector(r, 0.0f, 0.0f, 1.0f);
        }

        case ImageFormat::RGBA_Float:
        {
            const float* pSrc = (const float*)pData;
            return Vector(pSrc + 4 * (y * width + x));
        }
    }

    return Vector();
}

void SetTexel(const Vector& v, void* pData, uint32 x, uint32 y, uint32 width, ImageFormat fmt)
{
    switch (fmt)
    {
        case ImageFormat::A_UByte:
        {
            uchar* pSrc = (uchar*)pData;
            pSrc[y * width + x] = (uchar)(v.f[3] * 255.0f);
            break;
        }

        case ImageFormat::R_UByte:
        {
            uchar* pSrc = (uchar*)pData;
            pSrc[y * width + x] = (uchar)(v.f[0] * 255.0f);
            break;
        }

        case ImageFormat::RGB_UByte:
        {
            uchar* pSrc = (uchar*)pData;
            pSrc[3 * (y * width + x)] = (uchar)(v.f[0] * 255.0f);
            pSrc[3 * (y * width + x) + 1] = (uchar)(v.f[1] * 255.0f);
            pSrc[3 * (y * width + x) + 2] = (uchar)(v.f[2] * 255.0f);
            break;
        }

        case ImageFormat::RGBA_UByte:
        {
            uchar* pSrc = (uchar*)pData;
            pSrc[4 * (y * width + x)] = (uchar)(v.f[0] * 255.0f);
            pSrc[4 * (y * width + x) + 1] = (uchar)(v.f[1] * 255.0f);
            pSrc[4 * (y * width + x) + 2] = (uchar)(v.f[2] * 255.0f);
            pSrc[4 * (y * width + x) + 3] = (uchar)(v.f[3] * 255.0f);
            break;
        }

        case ImageFormat::R_Float:
        {
            float* pSrc = (float*)pData;
            pSrc[y * width + x] = v.f[0];
            break;
        }

        case ImageFormat::RGBA_Float:
        {
            Float4* pSrc = (Float4*)pData;
            VectorStore(v, pSrc + y * width + x);
            break;
        }
    }
}

} // namespace Common
} // namespace NFE
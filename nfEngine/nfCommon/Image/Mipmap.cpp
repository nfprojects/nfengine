/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Mipmap class definitions.
 */

#include "../PCH.hpp"
#include "Mipmap.hpp"
#include "../Logger.hpp"

namespace NFE {
namespace Common {

Mipmap::Mipmap()
    : mWidth(0)
    , mHeight(0)
{
}

Mipmap::Mipmap(void* data, uint32 width, uint32 height, size_t dataSize)
    : Mipmap()
{
    if (!SetData(data, width, height, dataSize))
        Release();
}

Mipmap::Mipmap(const Mipmap& other)
    : mWidth(other.mWidth)
    , mHeight(other.mHeight)
{
    mData.Load(other.mData.GetData(), other.mData.GetSize());
}

Mipmap::Mipmap(Mipmap&& other)
    : Mipmap()
{
    mData = other.mData;
    std::swap(mHeight, other.mHeight);
    std::swap(mWidth, other.mWidth);
}

Mipmap& Mipmap::operator=(const Mipmap& other)
{
    if (&other == this)
        return *this;

    mData.Load(other.mData.GetData(), other.mData.GetSize());

    mWidth = other.mWidth;
    mHeight = other.mHeight;
    return *this;
}

Mipmap::~Mipmap()
{
    Release();
}

void Mipmap::Release()
{
    if (!mData.GetData())
    {
        mData.Release();
    }

    mWidth = 0;
    mHeight = 0;
}

bool Mipmap::SetData(void* data, uint32 width, uint32 height, size_t dataSize)
{
    Release();
    mWidth = width;
    mHeight = height;
    mData.Load(data, dataSize);
    if (!mData.GetData())
    {
        LOG_ERROR("Allocation error for mData member.");
        return false;
    }

    return true;
}

const void* Mipmap::GetData() const
{
    if (const void* data = mData.GetData())
        return data;

    LOG_WARNING("mData member empty. Returned nullptr.");
    return nullptr;
}

uint32 Mipmap::GetWidth() const
{
    return mWidth;
}

uint32 Mipmap::GetHeight() const
{
    return mHeight;
}

size_t Mipmap::GetDataSize() const
{
    return mData.GetSize();
}

Color Mipmap::FilterBox(uint32 x, uint32 y, ImageFormat fmt)
{
    Color a = GetTexel(2 * x,     2 * y,     fmt);
    Color b = GetTexel(2 * x + 1, 2 * y,     fmt);
    Color c = GetTexel(2 * x,     2 * y + 1, fmt);
    Color d = GetTexel(2 * x + 1, 2 * y + 1, fmt);

    return ((a + b) + (c + d)) * 0.25f;
}

Color Mipmap::FilterGammaCorrected(uint32 x, uint32 y, ImageFormat fmt)
{
    Color a = GetTexel(2 * x,     2 * y,     fmt);
    Color b = GetTexel(2 * x + 1, 2 * y,     fmt);
    Color c = GetTexel(2 * x,     2 * y + 1, fmt);
    Color d = GetTexel(2 * x + 1, 2 * y + 1, fmt);
    a *= a;
    b *= b;
    c *= c;
    d *= d;

    return VectorSqrt(((a + b) + (c + d)) * 0.25f);
}

Color Mipmap::GetTexel(uint32 x, uint32 y, ImageFormat fmt) const
{
    switch (fmt)
    {
        case ImageFormat::A_UByte:
        {
            uchar* data = static_cast<uchar*>(mData.GetData());
            Float4 colors;
            colors.x = 1.0f;
            colors.y = 1.0f;
            colors.z = 1.0f;
            colors.w = static_cast<float>(data[(y * mWidth + x)]) / 255;
            return Color(colors);
        }

        case ImageFormat::R_UByte:
        {
            uchar* data = static_cast<uchar*>(mData.GetData());
            Float4 colors;
            colors.x = static_cast<float>(data[(y * mWidth + x)]) / 255;
            colors.y = 0.0f;
            colors.z = 0.0f;
            colors.w = 1.0f;
            return Color(colors);
        }

        case ImageFormat::RGB_UByte:
        {
            uchar* data = static_cast<uchar*>(mData.GetData());
            Float4 colors;
            colors.x = static_cast<float>(data[3 * (y * mWidth + x)]    ) / 255;
            colors.y = static_cast<float>(data[3 * (y * mWidth + x) + 1]) / 255;
            colors.z = static_cast<float>(data[3 * (y * mWidth + x) + 2]) / 255;
            colors.w = 1.0f;
            return Color(colors);
        }

        case ImageFormat::RGBA_UByte:
        {
            uchar* data = static_cast<uchar*>(mData.GetData());
            Float4 colors;
            colors.x = static_cast<float>(data[4 * (y * mWidth + x)]    ) / 255;
            colors.y = static_cast<float>(data[4 * (y * mWidth + x) + 1]) / 255;
            colors.z = static_cast<float>(data[4 * (y * mWidth + x) + 2]) / 255;
            colors.w = static_cast<float>(data[4 * (y * mWidth + x) + 3]) / 255;
            return Color(colors);
        }

        case ImageFormat::R_Float:
        {
            float* data = static_cast<float*>(mData.GetData());
            float r = data[y * mWidth + x];
            return Color(r, 0.0f, 0.0f, 1.0f);
        }

        case ImageFormat::RGBA_Float:
        {
            Float4* data = static_cast<Float4*>(mData.GetData());
            data += (y * mWidth + x);
            return Color(*data);
        }

        default:
        {
            LOG_WARNING("Image format %s not recognized, no texels gotten.", FormatToStr(fmt));
            return Color();
        }
    }
}

void Mipmap::SetTexel(const Color& v, uint32 x, uint32 y, ImageFormat fmt)
{
    switch (fmt)
    {
        case ImageFormat::A_UByte:
        {
            // TODO use function similar to VectorStoreUChar4
            uchar* data = static_cast<uchar*>(mData.GetData());
            data[y * mWidth + x] = static_cast<uchar>(v.f[3] * 255);
            break;
        }

        case ImageFormat::R_UByte:
        {
            // TODO use function similar to VectorStoreUChar4
            uchar* data = static_cast<uchar*>(mData.GetData());
            data[y * mWidth + x] = static_cast<uchar>(v.f[0] * 255);
            break;
        }

        case ImageFormat::RGB_UByte:
        {
            // TODO use function similar to VectorStoreUChar4
            uchar* data = static_cast<uchar*>(mData.GetData());
            data[3 * (y * mWidth + x)    ] = static_cast<uchar>(v.f[0] * 255);
            data[3 * (y * mWidth + x) + 1] = static_cast<uchar>(v.f[1] * 255);
            data[3 * (y * mWidth + x) + 2] = static_cast<uchar>(v.f[2] * 255);
            break;
        }

        case ImageFormat::RGBA_UByte:
        {
            uchar* data = static_cast<uchar*>(mData.GetData());
            data[4 * (y * mWidth + x)    ] = static_cast<uchar>(v.f[0] * 255);
            data[4 * (y * mWidth + x) + 1] = static_cast<uchar>(v.f[1] * 255);
            data[4 * (y * mWidth + x) + 2] = static_cast<uchar>(v.f[2] * 255);
            data[4 * (y * mWidth + x) + 3] = static_cast<uchar>(v.f[3] * 255);
            break;
        }

        case ImageFormat::R_Float:
        {
            float* data = static_cast<float*>(mData.GetData());
            data[y * mWidth + x] = v.f[0];
            break;
        }

        case ImageFormat::RGBA_Float:
        {
            Float4* data = static_cast<Float4*>(mData.GetData());
            data += (y * mWidth + x);
            VectorStore(v, data);
            break;
        }

        default:
        {
            LOG_WARNING("Image format %s not recognized, no texels set.", FormatToStr(fmt));
            break;
        }
    }
}

} // namespace Common
} // namespace NFE
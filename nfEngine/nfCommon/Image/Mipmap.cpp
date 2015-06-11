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

Mipmap::Mipmap() :
    mData(nullptr),
    mDataSize(0),
    mWidth(0),
    mHeight(0)
{
}

Mipmap::Mipmap(void* data, uint32 width, uint32 height, size_t dataSize)
    : Mipmap()
{
    if (SetData(data, width, height, dataSize))
        Close();
}

Mipmap::Mipmap(const Mipmap& other)
    :Mipmap(other.mData, other.mWidth, other.mHeight, other.mDataSize)
{
}

Mipmap::Mipmap(Mipmap&& other)
    :Mipmap()
{
    std::swap(mData, other.mData);
    std::swap(mDataSize, other.mDataSize);
    std::swap(mHeight, other.mHeight);
    std::swap(mWidth, other.mWidth);
}

Mipmap::~Mipmap()
{
    Close();
}

void Mipmap::Close()
{
    delete[] mData;
    mData = nullptr;
    mDataSize = 0;
    mWidth = 0;
    mHeight = 0;
}

const void* Mipmap::GetData() const
{
    if (mData)
        mData;

    LOG_WARNING("mData member empty. Returned std::nullptr.");
    return nullptr;
}

const uint32 Mipmap::GetWidth() const
{
    return mWidth;
}

const uint32 Mipmap::GetHeight() const
{
    return mHeight;
}

const size_t Mipmap::GetDataSize() const
{
    return mDataSize;
}

Color Mipmap::FilterBox(uint32 x, uint32 y)
{
    Color a = GetTexel(2 * x,     2 * y,     ImageFormat::RGBA_UByte);
    Color b = GetTexel(2 * x + 1, 2 * y,     ImageFormat::RGBA_UByte);
    Color c = GetTexel(2 * x,     2 * y + 1, ImageFormat::RGBA_UByte);
    Color d = GetTexel(2 * x + 1, 2 * y + 1, ImageFormat::RGBA_UByte);

    return ((a + b) + (c + d)) * 0.25f;
}

Color Mipmap::FilterGammaCorrected(uint32 x, uint32 y)
{
    Color a = GetTexel(2 * x,     2 * y,     ImageFormat::RGBA_UByte);
    Color b = GetTexel(2 * x + 1, 2 * y,     ImageFormat::RGBA_UByte);
    Color c = GetTexel(2 * x,     2 * y + 1, ImageFormat::RGBA_UByte);
    Color d = GetTexel(2 * x + 1, 2 * y + 1, ImageFormat::RGBA_UByte);
    a *= a;
    b *= b;
    c *= c;
    d *= d;

    return VectorSqrt(((a + b) + (c + d)) * 0.25f);
}


Vector Mipmap::Filter(MipmapFilter filter, uint32 x, uint32 y)
{
    switch (filter)
    {
    case MipmapFilter::Box:
        return FilterBox(x, y);
    case MipmapFilter::GammaCorrectedLinear:
        return FilterGammaCorrected(x, y);
    }

    LOG_WARNING("Could not operate on the given filter type : %s.", FilterToStr(filter));
    return Vector();
}

int Mipmap::SetData(void* data, uint32 width, uint32 height, size_t dataSize)
{
    Close();
    mWidth = width;
    mHeight = height;
    mDataSize = dataSize;

    mData = new (std::nothrow) uchar[dataSize];
    if (!mData)
    {
        LOG_ERROR("Allocation error for mData member.");
        return 1;
    }

    memcpy(mData, data, dataSize);
    return 0;
}

Vector Mipmap::GetTexel(uint32 x, uint32 y, ImageFormat fmt)
{
    switch (fmt)
    {
        case ImageFormat::A_UByte:
        {
            uchar a = mData[y * mWidth + x];
            return Vector(255.0f, 255.0f, 255.0f, (float)a) * g_Byte2Float;
        }

        case ImageFormat::R_UByte:
        {
            uchar r = mData[y * mWidth + x];
            return Vector((float)r, 0.0f, 0.0f, 255.0f) * g_Byte2Float;
        }

        case ImageFormat::RGB_UByte:
        {
            uchar r = mData[3 * (y * mWidth + x)];
            uchar g = mData[3 * (y * mWidth + x) + 1];
            uchar b = mData[3 * (y * mWidth + x) + 2];
            return Vector((float)r, (float)g, (float)b, 255.0f) * g_Byte2Float;
        }

        case ImageFormat::RGBA_UByte:
        {
            uchar r = mData[4 * (y * mWidth + x)];
            uchar g = mData[4 * (y * mWidth + x) + 1];
            uchar b = mData[4 * (y * mWidth + x) + 2];
            uchar a = mData[4 * (y * mWidth + x) + 3];
            return Vector((float)r, (float)g, (float)b, float(a)) * g_Byte2Float;
        }

        case ImageFormat::R_Float:
        {
            const float* fData = (const float*)mData;
            float r = fData[y * mWidth + x];
            return Vector(r, 0.0f, 0.0f, 1.0f);
        }

        case ImageFormat::RGBA_Float:
        {
            const float* fData = (const float*)mData;
            return Vector(fData + 4 * (y * mWidth + x));
        }
    }

    LOG_WARNING("Given image format not supported : %s", FormatToStr(fmt));
    return Vector();
}

bool Mipmap::SetTexel(const Color& v, uint32 x, uint32 y, ImageFormat fmt)
{
    switch (fmt)
    {
        case ImageFormat::A_UByte:
        {
            mData[y * mWidth + x] = (uchar)(v.f[3] * 255.0f);
            return true;
        }

        case ImageFormat::R_UByte:
        {
            mData[y * mWidth + x] = (uchar)(v.f[0] * 255.0f);
            return true;
        }

        case ImageFormat::RGB_UByte:
        {
            mData[3 * (y * mWidth + x)] = (uchar)(v.f[0] * 255.0f);
            mData[3 * (y * mWidth + x) + 1] = (uchar)(v.f[1] * 255.0f);
            mData[3 * (y * mWidth + x) + 2] = (uchar)(v.f[2] * 255.0f);
            return true;
        }

        case ImageFormat::RGBA_UByte:
        {
            mData[4 * (y * mWidth + x)] = (uchar)(v.f[0] * 255.0f);
            mData[4 * (y * mWidth + x) + 1] = (uchar)(v.f[1] * 255.0f);
            mData[4 * (y * mWidth + x) + 2] = (uchar)(v.f[2] * 255.0f);
            mData[4 * (y * mWidth + x) + 3] = (uchar)(v.f[3] * 255.0f);
            return true;
        }

        case ImageFormat::R_Float:
        {
            float* fData = (float*)mData;
            fData[y * mWidth + x] = v.f[0];
            return true;
        }

        case ImageFormat::RGBA_Float:
        {
            Float4* fData = (Float4*)mData;
            VectorStore(v, fData + y * mWidth + x);
            return true;
        }
    }

    LOG_WARNING("Given image format not supported : %s", FormatToStr(fmt));
    return false;
}

} // namespace Common
} // namespace NFE
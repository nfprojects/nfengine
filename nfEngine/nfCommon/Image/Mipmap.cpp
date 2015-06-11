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
    mWidth(0),
    mHeight(0)
{
}

Mipmap::Mipmap(void* data, uint32 width, uint32 height, size_t dataSize)
    : Mipmap()
{
    if (SetData(data, width, height, dataSize))
        Release();
}

Mipmap::Mipmap(const Mipmap& other)
{
    mData.reserve(other.mData.size());
    mData.assign(other.mData.begin(), other.mData.end());

    mWidth = other.mWidth;
    mHeight = other.mHeight;
}

Mipmap::Mipmap(Mipmap&& other)
    :Mipmap()
{
    std::swap(mData, other.mData);
    std::swap(mHeight, other.mHeight);
    std::swap(mWidth, other.mWidth);
}

Mipmap& Mipmap::operator=(const Mipmap& other)
{
    if (&other == this)
        return *this;

    mData.reserve(other.mData.size());
    mData.assign(other.mData.begin(), other.mData.end());

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
    if (!mData.empty())
    {
        mData.clear();
        mData.shrink_to_fit();
    }

    mWidth = 0;
    mHeight = 0;
}

const void* Mipmap::GetData() const
{
    if (!mData.empty())
        return mData.data();

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
    return mData.size();
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

bool Mipmap::SetData(void* data, uint32 width, uint32 height, size_t dataSize)
{
    Release();
    mWidth = width;
    mHeight = height;
    mData.reserve(dataSize);
    uchar* dataUchar = static_cast<uchar*>(data);
    mData.assign(dataUchar, dataUchar + dataSize);
    if (mData.empty())
    {
        LOG_ERROR("Allocation error for mData member.");
        return false;
    }

    return true;
}

Color Mipmap::GetTexel(uint32 x, uint32 y, ImageFormat fmt)
{
    switch (fmt)
    {
        case ImageFormat::A_UByte:
        {
            uchar colors[4];
            colors[0] = 255;
            colors[1] = 255;
            colors[2] = 255;
            colors[3] = mData[y * mWidth + x];
            return VectorLoadUChar4(colors);
        }

        case ImageFormat::R_UByte:
        {
            uchar colors[4];
            colors[0] = mData[y * mWidth + x];
            colors[1] = 0;
            colors[2] = 0;
            colors[3] = 255;
            return VectorLoadUChar4(colors);
        }

        case ImageFormat::RGB_UByte:
        {
            uchar colors[4];
            colors[0] = mData[3 * (y * mWidth + x)];
            colors[1] = mData[3 * (y * mWidth + x) + 1];
            colors[2] = mData[3 * (y * mWidth + x) + 2];
            colors[3] = 255;
            return VectorLoadUChar4(colors);
        }

        case ImageFormat::RGBA_UByte:
        {
            uchar colors[4];
            colors[0] = mData[4 * (y * mWidth + x)];
            colors[1] = mData[4 * (y * mWidth + x) + 1];
            colors[2] = mData[4 * (y * mWidth + x) + 2];
            colors[3] = mData[4 * (y * mWidth + x) + 3];
            return VectorLoadUChar4(colors);
        }

        case ImageFormat::R_Float:
        {
            float r = static_cast<float>(mData[y * mWidth + x]);
            return Color(r, 0.0f, 0.0f, 1.0f);
        }

        case ImageFormat::RGBA_Float:
        {
            float fData = static_cast<float>(mData[4 * (y * mWidth + x)]);
            return Color(fData);
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
            mData[y * mWidth + x] = static_cast<uchar>(v.f[3] * 255.0f);
            break;
        }

        case ImageFormat::R_UByte:
        {
            // TODO use function similar to VectorStoreUChar4
            mData[y * mWidth + x] = static_cast<uchar>(v.f[0] * 255.0f);
            break;
        }

        case ImageFormat::RGB_UByte:
        {
            // TODO use function similar to VectorStoreUChar4
            mData[3 * (y * mWidth + x)]     = static_cast<uchar>(v.f[0] * 255.0f);
            mData[3 * (y * mWidth + x) + 1] = static_cast<uchar>(v.f[1] * 255.0f);
            mData[3 * (y * mWidth + x) + 2] = static_cast<uchar>(v.f[2] * 255.0f);
            break;
        }

        case ImageFormat::RGBA_UByte:
        {
            uchar* destPtr = &mData[4 * (y * mWidth + x)];
            VectorStoreUChar4(v, destPtr);
            break;
        }

        case ImageFormat::R_Float:
        {
            float* fData = (float*)mData.data();
            fData[y * mWidth + x] = v.f[0];
            break;
        }

        case ImageFormat::RGBA_Float:
        {
            Float4* fData = (Float4*)mData.data();
            VectorStore(v, fData + y * mWidth + x);
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
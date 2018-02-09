/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Mipmap class definitions.
 */

#include "PCH.hpp"
#include "Mipmap.hpp"
#include "Logger/Logger.hpp"


namespace NFE {
namespace Common {

using namespace Math;

Mipmap::Mipmap()
    : mWidth(0)
    , mHeight(0)
{
}

Mipmap::Mipmap(const void* data, uint32 width, uint32 height, size_t dataSize)
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

bool Mipmap::SetData(const void* data, uint32 width, uint32 height, size_t dataSize)
{
    Release();
    mWidth = width;
    mHeight = height;
    mData.Load(data, dataSize);

    if (!mData.GetData())
    {
        NFE_LOG_ERROR("Allocation error for mData member.");
        return false;
    }

    return true;
}

const void* Mipmap::GetData() const
{
    const void* data = mData.GetData();
    if (data)
        return data;

    NFE_LOG_WARNING("mData member empty. Returned nullptr.");
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

    return Vector4::Sqrt(((a + b) + (c + d)) * 0.25f);
}

Color Mipmap::GetTexel(uint32 x, uint32 y, ImageFormat fmt) const
{
    switch (fmt)
    {
        case ImageFormat::A_UByte:
        {
            // TODO use function similar to VectorLoadUChar4
            uint8* data = static_cast<uint8*>(mData.GetData());
            Float4 colors;
            colors.x = 1.0f;
            colors.y = 1.0f;
            colors.z = 1.0f;
            colors.w = static_cast<float>(data[(y * mWidth + x)]) / 255.0f;
            return Color(colors);
        }

        case ImageFormat::R_UByte:
        {
            // TODO use function similar to VectorLoadUChar4
            uint8* data = static_cast<uint8*>(mData.GetData());
            Float4 colors;
            colors.x = static_cast<float>(data[(y * mWidth + x)]) / 255.0f;
            colors.y = 0.0f;
            colors.z = 0.0f;
            colors.w = 1.0f;
            return Color(colors);
        }

        case ImageFormat::RGB_UByte:
        {
            // TODO use function similar to VectorLoadUChar4
            uint8* data = static_cast<uint8*>(mData.GetData());
            Float4 colors;
            colors.x = static_cast<float>(data[3 * (y * mWidth + x)]    ) / 255.0f;
            colors.y = static_cast<float>(data[3 * (y * mWidth + x) + 1]) / 255.0f;
            colors.z = static_cast<float>(data[3 * (y * mWidth + x) + 2]) / 255.0f;
            colors.w = 1.0f;
            return Color(colors);
        }

        case ImageFormat::RGBA_UByte:
        {
            uint8* data = static_cast<uint8*>(mData.GetData());
            data += 4 * (y * mWidth + x);
            return Vector4::Load4(data) * VECTOR_INV_255;
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
            NFE_LOG_WARNING("Image format %s not recognized, no texels gotten.", FormatToStr(fmt));
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
            uint8* data = static_cast<uint8*>(mData.GetData());
            data[y * mWidth + x] = static_cast<uint8>(v.f[3] * 255.0f);
            break;
        }

        case ImageFormat::R_UByte:
        {
            // TODO use function similar to VectorStoreUChar4
            uint8* data = static_cast<uint8*>(mData.GetData());
            data[y * mWidth + x] = static_cast<uint8>(v.f[0] * 255.0f);
            break;
        }

        case ImageFormat::RGB_UByte:
        {
            // TODO use function similar to VectorStoreUChar4
            uint8* data = static_cast<uint8*>(mData.GetData());
            data[3 * (y * mWidth + x)    ] = static_cast<uint8>(v.f[0] * 255.0f);
            data[3 * (y * mWidth + x) + 1] = static_cast<uint8>(v.f[1] * 255.0f);
            data[3 * (y * mWidth + x) + 2] = static_cast<uint8>(v.f[2] * 255.0f);
            break;
        }

        case ImageFormat::RGBA_UByte:
        {
            uint8* data = static_cast<uint8*>(mData.GetData());
            data += 4 * (y * mWidth + x);
            (v * 255.0f).Store4(data);
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
            v.Store(data);
            break;
        }

        default:
        {
            NFE_LOG_WARNING("Image format %s not recognized, no texels set.", FormatToStr(fmt));
            break;
        }
    }
}

} // namespace Common
} // namespace NFE

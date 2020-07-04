/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Image class definitions.
 */

#include "PCH.hpp"
#include "Image.hpp"
#include "ImageType.hpp"

namespace NFE {
namespace Common {

Image::Image()
    : mWidth(0)
    , mHeight(0)
    , mFormat(ImageFormat::Unknown)
{
}

Image::Image(const Image& other)
    : mWidth(other.mWidth)
    , mHeight(other.mHeight)
    , mFormat(other.mFormat)
{
    // Clone all mMipmaps
    mMipmaps = other.mMipmaps;
}

Image::~Image()
{
    Release();
}

Image& Image::operator=(const Image& other)
{
    if (&other == this)
        return *this;

    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mFormat = other.mFormat;
    mMipmaps = other.mMipmaps;
    return *this;
}

Image& Image::operator=(Image&& other)
{
    std::swap(mFormat, other.mFormat);
    std::swap(mHeight, other.mHeight);
    std::swap(mWidth, other.mWidth);
    mMipmaps = std::move(other.mMipmaps);
    return *this;
}

void Image::ClearRegisteredTypesList()
{

}

void Image::Release()
{
    if (!mMipmaps.Empty())
    {
        for (auto& i : mMipmaps)
            i.Release();

        mMipmaps.Clear();
    }

    mWidth = 0;
    mHeight = 0;
    mFormat = ImageFormat::Unknown;
}

bool Image::SetData(const void* data, uint32 width, uint32 height, ImageFormat format)
{
    Release();
    size_t dataSize = (width * height * BitsPerPixel(format)) / 8;

    Mipmap mipmap(data, width, height, dataSize);

    if (!mipmap.GetData())
    {
        NFE_LOG_ERROR("Mipmap has not been created successfully.");
        return false;
    }

    mWidth = width;
    mHeight = height;
    mFormat = format;

    mMipmaps.PushBack(std::move(mipmap));

    return true;
}

bool Image::SetData(DynArray<Mipmap>&& mipmaps, uint32 width, uint32 height, ImageFormat format)
{
    Release();

    if (mipmaps.Empty())
    {
        NFE_LOG_ERROR("No mipmaps provided");
        return false;
    }

    mWidth = width;
    mHeight = height;
    mFormat = format;
    mMipmaps = std::move(mipmaps);

    return true;
}

int Image::GetWidth() const
{
    return mWidth;
}

int Image::GetHeight() const
{
    return mHeight;
}

const void* Image::GetData(uint32 mipmapId) const
{
    const Mipmap* mipmapPtr = GetMipmap(mipmapId);
    if (mipmapPtr)
        return mipmapPtr->GetData();

    return nullptr;
}

ImageFormat Image::GetFormat() const
{
    return mFormat;
}

uint32 Image::GetMipmapsNum() const
{
    return mMipmaps.Size();
}

void Image::SetTexel(const Color& v, uint32 x, uint32 y)
{
    if (!mMipmaps.Empty())
    {
        mMipmaps[0].SetTexel(v, x, y, mFormat);
    }
}

const Mipmap* Image::GetMipmap(uint32 id) const
{
    if (id < mMipmaps.Size())
        return &mMipmaps[id];

    NFE_LOG_WARNING("Trying to access not existing mipmap.");
    return nullptr;
}

} // namespace Common
} // namespace NFE

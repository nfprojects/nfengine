/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Image class definitions.
 */

#include "PCH.hpp"
#include "Image.hpp"
#include "Logger/Logger.hpp"
#include "squish/src/squish.h"


namespace {

// TODO After BC6H-BC7 implementation refactor gDDSSrcFormat
/**
 * Source format, from which DDS conversion is made
 */
const NFE::Common::ImageFormat gDDSSrcFormat = NFE::Common::ImageFormat::RGBA_UByte;
}

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
    mMipmaps.reserve(other.mMipmaps.size());
    mMipmaps.assign(other.mMipmaps.begin(), other.mMipmaps.end());
}

Image::~Image()
{
    Release();
}

ImageTypeMap& Image::mImageTypes()
{
    static ImageTypeMap mImageType;
    return mImageType;
}

void Image::Release()
{
    if (!mMipmaps.empty())
    {
        for (auto& i : mMipmaps)
            i.Release();

        mMipmaps.clear();
        mMipmaps.shrink_to_fit();
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

    mMipmaps.push_back(mipmap);

    return true;
}

bool Image::Load(InputStream* stream)
{
    Release();

    for (const auto &i : mImageTypes())
    {
        if (i.second->Check(stream))
            return i.second->Load(this, stream);
    }

    NFE_LOG_ERROR("Stream was not recognized as any of the usable file formats.");
    return false;
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

size_t Image::GetMipmapsNum() const
{
    return mMipmaps.size();
}

const Mipmap* Image::GetMipmap(uint32 id) const
{
    if (id < mMipmaps.size())
        return &mMipmaps[id];

    NFE_LOG_WARNING("Trying to access not existing mipmap.");
    return nullptr;
}

bool Image::DecompressDDS()
{
    // Find compression flag
    int compressionFlag = 0;
    switch (mFormat)
    {
    case ImageFormat::BC1:
        compressionFlag = squish::kDxt1;
        break;
    case ImageFormat::BC2:
        compressionFlag = squish::kDxt3;
        break;
    case ImageFormat::BC3:
        compressionFlag = squish::kDxt5;
        break;
    default:
        NFE_LOG_WARNING("BC Decompression cannot be done for %s pixel format.",
            FormatToStr(mFormat));
        return false;
    }

    // Calculate array sizes and allocate the memory
    const int size = static_cast<int>(mWidth * mHeight
                                        * BitsPerPixel(gDDSSrcFormat) / sizeof(uint8));

    std::unique_ptr<uint8[]> pixels(new (std::nothrow) uint8[size]);
    if (!pixels.get())
    {
        NFE_LOG_ERROR("Allocating memory for DDS decompression failed.");
        return false;
    }

    const uint8* block = static_cast<const uint8*>(GetData());

    // Decompress the first mipmap
    squish::DecompressImage(pixels.get(), mWidth, mHeight, block, compressionFlag);
    return SetData(pixels.get(), mWidth, mHeight, gDDSSrcFormat);
}

bool Image::CompressDDS(ImageFormat destFormat)
{
    if (mFormat != gDDSSrcFormat)
    {
        // TODO After BC6H-BC7 implementation refactor gDDSSrcFormat
        NFE_LOG_WARNING("Compression to BC1-3 can only be made for %s pixel format.",
            FormatToStr(gDDSSrcFormat));
        return false;
    }

    // Find compression flag
    int compressionFlag = 0;
    switch (destFormat)
    {
    case ImageFormat::BC1:
        compressionFlag = squish::kDxt1;
        break;
    case ImageFormat::BC2:
        compressionFlag = squish::kDxt3;
        break;
    case ImageFormat::BC3:
        compressionFlag = squish::kDxt5;
        break;
    default:
        NFE_LOG_WARNING("BC Compression cannot be done for %s pixel format.",
            FormatToStr(destFormat));
        return false;
    }

    std::vector<Mipmap> newMipmaps;
    // Get number of mipmaps to be compressed, apart from the 1st one
    uint32 mipmapNum = static_cast<uint32>(GetMipmapsNum());

    // Calculate array sizes and allocate the memory
    size_t bitsPerPixel = BitsPerPixel(destFormat);
    const Mipmap* mip = GetMipmap(0);
    uint32 width = mip->GetWidth();
    uint32 height = mip->GetHeight();
    size_t size = width * height * bitsPerPixel;

    const uint8* pixels = static_cast<const uint8*>(mip->GetData());
    std::unique_ptr<uint8[]> block(new (std::nothrow) uint8[size / sizeof(uint8)]);
    if (!block.get())
    {
        NFE_LOG_ERROR("Allocating memory for DDS compression failed.");
        return false;
    }

    // Compress the first mipmap
    squish::CompressImage(pixels, width, height, block.get(), compressionFlag);
    newMipmaps.emplace_back(Mipmap(block.get(), width, height, size));

    // Compress the rest
    for (uint32 i = 1; i < mipmapNum; i++)
    {
        mip = GetMipmap(i);
        width = mip->GetWidth();
        height = mip->GetHeight();

        if (width < 4 || height < 4)
            break;
        size = width * height * bitsPerPixel;

        pixels = static_cast<const uint8*>(mip->GetData());
        block.reset(new (std::nothrow) uint8[size / sizeof(uint8)]);
        if (!block.get())
        {
            NFE_LOG_ERROR("Compressing mipmap of level %d failed.", i);
            return false;
        }

        squish::CompressImage(pixels, width, height, block.get(), compressionFlag);
        newMipmaps.emplace_back(Mipmap(block.get(), width, height, size));
    }

    Release();
    mWidth = width;
    mHeight = height;
    mFormat = destFormat;
    mMipmaps.assign(newMipmaps.begin(), newMipmaps.end());

    return true;
}

bool Image::GenerateMipmaps(MipmapFilter filterType, uint32 num)
{
    // Empty image
    if (!GetData())
    {
        NFE_LOG_WARNING("Tried to generate mMipmaps of an empty image");
        return false;
    }

    if (mFormat == ImageFormat::BC4 || mFormat == ImageFormat::BC5 ||
        mFormat == ImageFormat::BC6H || mFormat == ImageFormat::BC7)
    {
        NFE_LOG_ERROR("Block coded (BC4-BC7) pixel formats are unsupported");
        return false;
    }

    if (mFormat == ImageFormat::Unknown)
    {
        NFE_LOG_ERROR("Invalid pixel format");
        return false;
    }

    ImageFormat oldFormat = ImageFormat::Unknown;
    if (IsSupportedBC(mFormat))
    {
        oldFormat = mFormat;
        if (!Convert(gDDSSrcFormat))
        {
            NFE_LOG_ERROR("%s decompression failed. No mipmaps generated.",
                        FormatToStr(oldFormat));
            return false;
        }
    }

    if (!GenerateMipmapsActual(filterType, num))
        return false;

    if (oldFormat != ImageFormat::Unknown)
    {
        if (!Convert(oldFormat))
        {
            NFE_LOG_ERROR("%s compression failed. Mipmaps generated in %s format.",
                        FormatToStr(oldFormat), FormatToStr(mFormat));
            return false;
        }
    }

    return true;
}

bool Image::GenerateMipmapsActual(MipmapFilter filterType, uint32 num)
{
    // Erase all existing mipmaps apart from the first one
    mMipmaps.resize(1);
    mMipmaps.shrink_to_fit();

    // Start from most detailed mipmap
    Mipmap* mipmap;
    Mipmap::filterFunctor filter = nullptr;

    // Resolve what filter should be used
    switch (filterType)
    {
    case MipmapFilter::GammaCorrectedLinear:
        filter = &Mipmap::FilterGammaCorrected;
        break;

    case MipmapFilter::Box:
        filter = &Mipmap::FilterBox;
        break;

    default:
        NFE_LOG_ERROR("Unknown filter specified for mipmap generation: %s.",
            FilterToStr(filterType));
        return false;
    }

    // TODO  NEEDS CHANGE
    auto isPowerOfTwo = [](int x){return !(x == 0) && !(x & (x - 1)); };
    if (isPowerOfTwo(mMipmaps.back().GetWidth())
        || isPowerOfTwo(mMipmaps.back().GetHeight()))
        NFE_LOG_WARNING("No support for non-power-of-2 !");

    // Generate mipmaps main loop
    for (uint32 i = 0; i < num; i++)
    {
        mipmap = &mMipmaps.back();

        // Calculate the size of the new mipmap
        uint32 nextMipWidth = (mipmap->GetWidth() / 2) + (mipmap->GetWidth() % 2);
        uint32 nextMipHeight = (mipmap->GetHeight() / 2) + (mipmap->GetHeight() % 2);
        if (nextMipWidth == 0)
            nextMipWidth = 1;
        if (nextMipHeight == 0)
            nextMipHeight = 1;
        size_t dataSize = nextMipWidth * nextMipHeight * BitsPerPixel(mFormat);

        // Make empty data for the new mipmap so it allocates the space needed
        std::unique_ptr<uint8[]> data(new (std::nothrow) uint8[dataSize / sizeof(uint8)]);
        if (!data.get())
        {
            NFE_LOG_ERROR("Allocating memory for mipmap generation failed.");
            return false;
        }

        Mipmap nextMip(data.get(), nextMipWidth, nextMipHeight, dataSize);

        // Iterate through whole mipmap and acquire its texels
        for (uint32 y = 0; y < nextMipHeight; y++)
        {
            for (uint32 x = 0; x < nextMipWidth; x++)
            {
                Color outputTexel = (mipmap->*filter)(x, y, mFormat);
                nextMip.SetTexel(outputTexel, x, y, mFormat);
            }
        }
        mMipmaps.push_back(nextMip);

        if (nextMipWidth == 1 && nextMipHeight == 1)
            break;
    }

    return true;
}

bool Image::Convert(ImageFormat destFormat)
{
    // Empty image
    if (GetData() == 0)
    {
        NFE_LOG_WARNING("Tried to convert pixel format of an empty image");
        return false;
    }

    if (mFormat == ImageFormat::BC4 || mFormat == ImageFormat::BC5 ||
        mFormat == ImageFormat::BC6H || mFormat == ImageFormat::BC7 ||
        destFormat == ImageFormat::BC4 || destFormat == ImageFormat::BC5 ||
        destFormat == ImageFormat::BC6H || destFormat == ImageFormat::BC7)
    {
        NFE_LOG_ERROR("Block coded (BC4-BC7) pixel formats are currently not supported");
        return false;
    }

    // Invalid format
    if (mFormat == ImageFormat::Unknown || destFormat == ImageFormat::Unknown)
    {
        NFE_LOG_ERROR("Trying to convert invalid pixel format");
        return false;
    }

    // Same format
    if (destFormat == mFormat)
        return true;

    if (IsSupportedBC(mFormat))
    {
        uint32 mipmapNum = static_cast<uint32>(GetMipmapsNum() - 1);
        DecompressDDS();
        GenerateMipmaps(MipmapFilter::Box, mipmapNum);

        if (IsSupportedBC(destFormat))
            return CompressDDS(destFormat);
        else
            return ConvertActual(destFormat);
    }

    if (IsSupportedBC(destFormat))
    {
        if (mFormat != gDDSSrcFormat)
            ConvertActual(gDDSSrcFormat);
        return CompressDDS(destFormat);
    }
    return ConvertActual(destFormat);
}

bool Image::ConvertActual(ImageFormat destFormat)
{
    std::vector<Mipmap> newMipmaps;
    newMipmaps.resize(mMipmaps.size());

    for (uint32 i = 0; i < mMipmaps.size(); i++)
    {
        uint32 width = mMipmaps[i].GetWidth();
        uint32 height = mMipmaps[i].GetHeight();
        newMipmaps[i].mData.Create(width * height * BitsPerPixel(destFormat));
        newMipmaps[i].mHeight = height;
        newMipmaps[i].mWidth = width;

        Math::Vector tmp;

        for (uint32 y = 0; y < height; y++)
        {
            for (uint32 x = 0; x < width; x++)
            {
                tmp = mMipmaps[i].GetTexel(x, y, mFormat);
                newMipmaps[i].SetTexel(tmp, x, y, destFormat);
            }
        }
    }
    mMipmaps.assign(newMipmaps.begin(), newMipmaps.end());

    mFormat = destFormat;

    return true;
}

bool Image::Grayscale()
{
    if (mFormat == ImageFormat::RGB_UByte
        || mFormat == ImageFormat::RGBA_UByte
        || mFormat == ImageFormat::RGBA_Float)
    {
        // These are common values for luminance grayscale transformation
        Color luminanceMask(0.21f, 0.72f, 0.07f, 1.0f);

        for (uint32 i = 0; i < mMipmaps.size(); i++)
        {
            uint32 width = mMipmaps[i].GetWidth();
            uint32 height = mMipmaps[i].GetHeight();

            Color tmp;

            for (uint32 y = 0; y < height; y++)
            {
                for (uint32 x = 0; x < width; x++)
                {
                    // Get texel, so we can calculate dot product and use its alpha
                    tmp = mMipmaps[i].GetTexel(x, y, mFormat);

                    // Calculate grayscale value using luminance transformation
                    float grayVal = Math::Vector::Dot3(tmp, luminanceMask);

                    // Overwrite only color values, leaving alpha as it was
                    mMipmaps[i].SetTexel(Color(grayVal, grayVal, grayVal, tmp.f[3]),
                                           x, y, mFormat);
                }
            }
        }

        return true;
    }

    NFE_LOG_ERROR("Conversion to grayscale is not available for image format: %s.",
              FormatToStr(mFormat));
    return false;
}
} // namespace Common
} // namespace NFE

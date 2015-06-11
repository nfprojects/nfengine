/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Image class definitions.
 */

#include "../PCH.hpp"
#include "Image.hpp"
#include "../Logger.hpp"
#include "ImageFormat.hpp"
#include "squish/squish.h"

namespace NFE {
namespace Common {

Image::Image()
{
    mWidth = 0;
    mHeight = 0;
    mFormat = ImageFormat::Unknown;
}

Image::Image(const Image& other)
{
    Release();

    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mFormat = other.mFormat;

    //clone all mMipmaps
    mMipmaps.reserve(other.mMipmaps.size());
    mMipmaps.assign(other.mMipmaps.begin(), other.mMipmaps.end());
}

Image::~Image()
{
    Release();
}

void Image::Release()
{
    if (!mMipmaps.empty())
    {
        for (auto i : mMipmaps)
            i.Release();

        mMipmaps.clear();
        mMipmaps.shrink_to_fit();
    }

    mWidth = 0;
    mHeight = 0;
}

int Image::SetData(void* pData, uint32 width, uint32 height, ImageFormat format)
{
    Release();
    size_t dataSize = width * height * BitsPerPixel(format) / 8;
    Mipmap mipmap(pData, width, height, dataSize);

    if (!mipmap.GetData())
    {
        LOG_ERROR("Mipmap has not been created successfully.")
        return 1;
    }

    mWidth = width;
    mHeight = height;
    mFormat = format;

    mMipmaps.push_back(mipmap);

    return 0;
}

int Image::Load(InputStream* pStream)
{
    pStream->Seek(0);
    if (!LoadBMP(pStream))
        return 0;

    pStream->Seek(0);
    if (!LoadPNG(pStream))
        return 0;

    pStream->Seek(0);
    if (!LoadJPG(pStream))
        return 0;

    pStream->Seek(0);
    if (!LoadDDS(pStream))
        return 0;

    Release();
    LOG_WARNING("Stream was not recognized as any of the usable pixel formats.")
    return 1;
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

    LOG_WARNING("Trying to access not existing mipmap.");
    return nullptr;
}

int Image::DecompressDDS()
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
        LOG_WARNING("BC Decompression cannot be done for %s pixel format.",
            FormatToStr(mFormat));
        return 1;
    }

    // Calculate array sizes and allocate the memory
    const int size = static_cast<int>(mWidth * mHeight * BitsPerPixel(ImageFormat::RGBA_UByte) / 8);

    std::unique_ptr<uchar[]> pixels(new uchar[size]);
    const uchar* block = static_cast<const uchar*>(GetData());

    // Decompress the first mipmap
    squish::DecompressImage(pixels.get(), mWidth, mHeight, block, compressionFlag);
    return SetData(pixels.get(), mWidth, mHeight, ImageFormat::RGBA_UByte);
}

int Image::CompressDDS(ImageFormat destFormat)
{
    if (mFormat != ImageFormat::RGBA_UByte)
    {
        LOG_WARNING("Compression to BC1-3 can only be made for %s pixel format.",
            FormatToStr(ImageFormat::RGBA_UByte));
        return 1;
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
        LOG_WARNING("BC Compression cannot be done for %s pixel format.",
            FormatToStr(destFormat));
        return 1;
    }

    // Get number of mipmaps to be compressed, apart from the 1st one
    uint32 mipmapNum = static_cast<uint32>(GetMipmapsNum());

    // Calculate array sizes and allocate the memory
    uint32 bitsPerPixel = static_cast<uint32>(BitsPerPixel(destFormat));
    const Mipmap* mip = GetMipmap(0);
    uint32 width = mip->GetWidth();
    uint32 height = mip->GetHeight();
    uint32 size = width * height * bitsPerPixel / 8;

    const uchar* pixels = static_cast<const uchar*>(mip->GetData());
    std::unique_ptr<uchar[]> block(new uchar[size]);
    
    // Compress the first mipmap
    squish::CompressImage(pixels, width, height, block.get(), compressionFlag);
    if (SetData(block.get(), width, height, destFormat))
    {
        LOG_ERROR("Storing first mipmap failed.");
        return 1;
    }

    // Compress the rest
    for (uint32 i = 1; i < mipmapNum; i++)
    {
        mip = GetMipmap(i);
        width = mip->GetWidth();
        height = mip->GetHeight();
        size = width * height * bitsPerPixel / 8;

        pixels = static_cast<const uchar*>(mip->GetData());
        block.reset(new uchar[size]);

        squish::CompressImage(pixels, width, height, block.get(), compressionFlag);
        mMipmaps.push_back(Mipmap(block.get(), width, height, size));
    }

    return 0;
}

int Image::GenerateMipmaps(MipmapFilter filterType, uint32 num)
{
    //empty image
    if (!GetData())
    {
        LOG_WARNING("Tried to generate mMipmaps of an empty image");
        return 0;
    }

    if (mFormat == ImageFormat::BC4 || mFormat == ImageFormat::BC5 ||
        mFormat == ImageFormat::BC6H || mFormat == ImageFormat::BC7)
    {
        LOG_ERROR("Block coded (BC4-BC7) pixel formats are unsupported");
        return 1;
    }

    if (mFormat == ImageFormat::Unknown)
    {
        LOG_ERROR("Invalid pixel format");
        return 1;
    }

    ImageFormat oldFormat = ImageFormat::Unknown;
    if (mFormat == ImageFormat::BC1 || mFormat == ImageFormat::BC2 || mFormat == ImageFormat::BC3)
    {
        oldFormat = mFormat;
        if (!Convert(ImageFormat::RGBA_UByte))
        {
            LOG_ERROR("%s decompression failed. No mipmaps generated.",
                        FormatToStr(oldFormat));
            return 1;
        }
    }

    GenerateMipmapsActual(filterType, num);

    if (oldFormat != ImageFormat::Unknown)
    {
        if (!Convert(oldFormat))
        {
            LOG_ERROR("%s compression failed. Mipmaps generated in %s format.",
                        FormatToStr(oldFormat), FormatToStr(mFormat));
            return 1;
        }
    }

    return 0;
}

int Image::GenerateMipmapsActual(MipmapFilter filterType, uint32 num)
{
    // Erase all existing mipmaps apart from the first one
    mMipmaps.resize(1);
    mMipmaps.shrink_to_fit();

    // Start from most detailed mipmap
    Mipmap* mipmap;

    // Resolve what filter should be used
    Mipmap::filterFunctor filterFunction = nullptr;
    
    switch (filterType)
    {
    case MipmapFilter::GammaCorrectedLinear:
        filterFunction = std::bind(&Mipmap::FilterGammaCorrected, mipmap, std::placeholders::_1,
                                    std::placeholders::_2, std::placeholders::_3);
        break;

    case MipmapFilter::Box:
    default:
        filterFunction = std::bind(&Mipmap::FilterBox, mipmap, std::placeholders::_1,
                                    std::placeholders::_2, std::placeholders::_3);
        break;
    }
    
    // Generate mipmaps main loop
    for (uint32 i = 0; i < num; i++)
    {
        mipmap = &mMipmaps.back();
        //TODO  NEEDS CHANGE - NO SUPPORT FOR non-power-of-2 !!!!

        // Calculate the size of the new mipmap
        uint32 nextMipWidth = (mipmap->GetWidth() / 2) + (mipmap->GetWidth() % 2);
        uint32 nextMipHeight = (mipmap->GetHeight() / 2) + (mipmap->GetHeight() % 2);
        if (nextMipWidth == 0)
            nextMipWidth = 1;
        if (nextMipHeight == 0)
            nextMipHeight = 1;
        size_t dataSize = nextMipWidth * nextMipHeight * BitsPerPixel(mFormat) / 8;

        // Make empty data for the new mipmap so it allocates the space needed
        std::unique_ptr<uchar[]> data(new (std::nothrow) uchar[dataSize]);
        Mipmap nextMip(data.get(), nextMipWidth, nextMipHeight, dataSize);

        // Iterate through whole mipmap and acquire its texels
        for (uint32 y = 0; y < nextMipHeight; y++)
        {
            for (uint32 x = 0; x < nextMipWidth; x++)
            {
                Color outputTexel = filterFunction(x, y, mFormat);
                nextMip.SetTexel(outputTexel, x, y, mFormat);
            }
        }
        mMipmaps.push_back(nextMip);

        if (nextMipWidth == 1 && nextMipHeight == 1)
            break;
    }

    return 0;
}

int Image::Convert(ImageFormat destFormat)
{
    // empty image
    if (GetData() == 0)
    {
        LOG_WARNING("Tried to convert pixel format of an empty image");
        return 0;
    }

    if (mFormat == ImageFormat::BC4 || mFormat == ImageFormat::BC5 ||
        mFormat == ImageFormat::BC6H || mFormat == ImageFormat::BC7 ||
        destFormat == ImageFormat::BC4 || destFormat == ImageFormat::BC5 ||
        destFormat == ImageFormat::BC6H || destFormat == ImageFormat::BC7)
    {
        LOG_ERROR("Block coded (BC4-BC7) pixel formats are currently impossible to convert");
        return 1;
    }

    //Invalid format
    if (mFormat == ImageFormat::Unknown || destFormat == ImageFormat::Unknown)
    {
        LOG_ERROR("Trying to convert invalid pixel format");
        return 1;
    }

    // same format
    if (destFormat == mFormat)
        return 0;

    if (mFormat == ImageFormat::BC1 || mFormat == ImageFormat::BC2 || mFormat == ImageFormat::BC3)
    {
        uint32 mipmapNum = static_cast<uint32>(GetMipmapsNum());
        DecompressDDS();
        GenerateMipmaps(MipmapFilter::Box, mipmapNum);

        if (destFormat == ImageFormat::BC1 || destFormat == ImageFormat::BC2 || destFormat == ImageFormat::BC3)
            return CompressDDS(destFormat);
        else
            return ConvertActual(destFormat);
    }

    if (destFormat == ImageFormat::BC1 || destFormat == ImageFormat::BC2 || destFormat == ImageFormat::BC3)
    {
        if (mFormat != ImageFormat::RGBA_UByte)
            ConvertActual(ImageFormat::RGBA_UByte);
        return CompressDDS(destFormat);
    } else
        return ConvertActual(destFormat);
}

int Image::ConvertActual(ImageFormat destFormat)
{
    std::vector<Mipmap> newMipmaps;
    newMipmaps.resize(mMipmaps.size());

    for (int i = 0; i < mMipmaps.size(); i++)
    {
        //allocate buffer for new format
        uint32 width = mMipmaps[i].GetWidth();
        uint32 height = mMipmaps[i].GetHeight();

        Vector tmp;

        for (uint32 y = 0; y < height; y++)
        {
            for (uint32 x = 0; x < width; x++)
            {
                tmp = mMipmaps[i].GetTexel(x, y, mFormat);
                if (!newMipmaps[i].SetTexel(tmp, x, y, destFormat))
                {
                    LOG_ERROR("Setting texel (%d, %d) for %s pixel format failed.",
                                x, y, FormatToStr(destFormat));
                    return 1;
                }
            }
        }
    }
    mMipmaps.assign(newMipmaps.begin(), newMipmaps.end());

    mFormat = destFormat;

    return 0;
}

} //namespace Common
} // namespace NFE

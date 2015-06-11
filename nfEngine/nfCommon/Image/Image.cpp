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
#include "ImageConverter.hpp"

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
            i.Close();

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
    /*
    int result;

    auto loadLambda = [&](std::function<int (InputStream*)> loadFunc) -> int
    {
        pStream->Seek(0);
        result = loadFunc(pStream);
        if (!result)
            return result;
    };

    loadLambda(&LoadBMP);
    loadLambda(&LoadPNG);
    loadLambda(&LoadJPEG);
    loadLambda(&LoadDDS);
    */
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
    if (mMipmaps.empty())
    {
        LOG_WARNING("Trying to access not existing mipmap.");
        return nullptr;
    }

    if (id < mMipmaps.size())
        return &mMipmaps[id];

    LOG_WARNING("Trying to access not existing mipmap.");
    return nullptr;
}

int Image::GenerateMipmaps(MipmapFilter filterType, uint32 num)
{
    //empty image
    if (!GetData())
    {
        LOG_WARNING("Tried to generate mMipmaps of an empty image");
        return 0;
    }

    if (mFormat == ImageFormat::BC2 || mFormat == ImageFormat::BC3 ||
            mFormat == ImageFormat::BC4 || mFormat == ImageFormat::BC5)
    {
        //TODO convert to RGBA, rerun this func, convert back to bc
        LOG_WARNING("Block coded (BCx) pixel formats are unsupported");
        return 1;
    }

    if (mFormat == ImageFormat::Unknown)
    {
        LOG_WARNING("Invalid pixel format");
        return 1;
    }

    // Erase all existing mipmaps apart from the first one
    mMipmaps.resize(1);
    mMipmaps.shrink_to_fit();

    if (mFormat == ImageFormat::BC1)
    {
        ImageFormat oldFormat = mFormat;
        if (!mMipmaps.back().DecompressDDS(oldFormat))
        {
            LOG_WARNING("DDS decompression failed. No mipmaps generated.");
            return 1;
        }
        mFormat = ImageFormat::RGBA_UByte;
        GenerateMipmaps(filterType, num);
        
        for (auto i : mMipmaps)
            if (!i.CompressDDS(oldFormat))
            {
                LOG_WARNING("DDS compression failed. Mipmaps generated in %s format.",
                            FormatToStr(ImageFormat::RGBA_UByte));
                return 1;
            }
        mFormat = oldFormat;
    }

    // Start from most detailed mipmap
    Mipmap* pMip;

    for (uint32 i = 0; i < num; i++)
    {
        pMip = &mMipmaps.back();
        //TODO  NEEDS CHANGE - NO SUPPORT FOR non-power-of-2 !!!!
        
        // Calculate the size of the new mipmap
        uint32 nextMipWidth = (pMip->GetWidth() / 2) + (pMip->GetWidth() % 2);
        uint32 nextMipHeight = (pMip->GetHeight() / 2) + (pMip->GetHeight() % 2);
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
                Color outputTexel = pMip->Filter(filterType, x, y);
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
    // same format
    if (destFormat == mFormat)
        return 0;

    // empty image
    if (GetData() == 0)
    {
        LOG_WARNING("Tried to convert pixel format of an empty image");
        return 0;
    }

    // BC conversion - TODO!
    if (mFormat == ImageFormat::BC2 || mFormat == ImageFormat::BC3 ||
        mFormat == ImageFormat::BC4 || mFormat == ImageFormat::BC5 ||
        destFormat == ImageFormat::BC2 || destFormat == ImageFormat::BC3 ||
        destFormat == ImageFormat::BC4 || destFormat == ImageFormat::BC5)
    {
        LOG_WARNING("Block coded (BCx) pixel formats other than BC1 are currently impossible to convert"); // TODO
        return 1;
    }

    //Invalid format
    if (mFormat == ImageFormat::Unknown || destFormat == ImageFormat::Unknown)
    {
        LOG_WARNING("Trying to convert invalid pixel format");
        return 1;
    }

    if (mFormat == ImageFormat::BC1)
        for (auto i : mMipmaps)
            i.DecompressDDS(mFormat);

    if (destFormat == ImageFormat::BC1)
    {
        Convert(ImageFormat::RGBA_UByte);
        for (auto i : mMipmaps)
            i.CompressDDS(destFormat);
    }
    /*
    Mipmap imageToConvert = mMipmaps.front();
    void* data = const_cast<void*>(imageToConvert.GetData());
    ImageConverter converter(data, imageToConvert.GetDataSize());
    if (!converter.Convert(destFormat))
    {
        LOG_WARNING("Conversion failed.");
        return 1;
    }
    */
    std::vector<Mipmap> newMipmaps;
    newMipmaps.resize(mMipmaps.size);

    for (int i = 0; i < mMipmaps.size; i++)
    {
        //alloc buffer for new format
        uint32 width = mMipmaps[i].GetWidth();
        uint32 height = mMipmaps[i].GetHeight();
        
        Vector tmp;

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

    return 0;
}

} //namespace Common
} // namespace NFE

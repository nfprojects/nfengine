/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Image function definitions.
 */

#include "PCH.hpp"
#include "Image.hpp"
#include "Math/Math.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "Logger.hpp"

namespace NFE {
namespace Common {

using namespace Math;

Image::Image()
{
    mWidth = 0;
    mHeight = 0;
    mFormat = ImageFormat::Unknown;
}

Image::Image(const Image& src)
{
    mWidth = src.mWidth;
    mHeight = src.mHeight;
    mFormat = src.mFormat;

    //clone all mMipmaps
    for (size_t i = 0; i < src.mMipmaps.size(); i++)
    {
        ImageMipmap mipmap;
        mipmap.width = src.mMipmaps[i].width;
        mipmap.height = src.mMipmaps[i].height;
        mipmap.dataSize = src.mMipmaps[i].dataSize;
        mipmap.data = malloc(mipmap.dataSize);

        if (mipmap.data == 0)
            return;

        memcpy(mipmap.data, src.mMipmaps[i].data, mipmap.dataSize);
        mMipmaps.push_back(mipmap);
    }
}

Image::~Image()
{
    Release();
}

void Image::Release()
{
    for (uint32 i = 0; i < mMipmaps.size(); i++)
        free(mMipmaps[i].data);

    mMipmaps.clear();
    mWidth = 0;
    mHeight = 0;
}

int Image::SetData(void* pData, int width, int height, ImageFormat format)
{
    Release();

    ImageMipmap mipmap;
    mipmap.width = width;
    mipmap.height = height;
    mipmap.dataSize = width * height * BitsPerPixel(format) / 8;
    mipmap.data = malloc(mipmap.dataSize);

    if (mipmap.data == 0)
        return 1;

    mWidth = width;
    mHeight = height;
    mFormat = format;
    memcpy(mipmap.data, pData, mipmap.dataSize);
    mMipmaps.push_back(mipmap);

    return 0;
}

int Image::Load(InputStream* pStream)
{
    int result;

    pStream->Seek(0);
    result = LoadBMP(pStream);
    if (result == 0) return result;

    pStream->Seek(0);
    result = LoadPNG(pStream);
    if (result == 0) return result;

    pStream->Seek(0);
    result = LoadJPEG(pStream);
    if (result == 0) return result;

    pStream->Seek(0);
    result = LoadDDS(pStream);
    if (result == 0) return result;

    Release();
    return result;
}

int Image::GetWidth() const
{
    return mWidth;
}

int Image::GetHeight() const
{
    return mHeight;
}

void* Image::GetData(uint32 mipmap) const
{
    if (mipmap < mMipmaps.size())
        return mMipmaps[mipmap].data;

    return 0;
}

ImageFormat Image::GetFormat() const
{
    return mFormat;
}

size_t Image::GetMipmapsNum() const
{
    return mMipmaps.size();
}

const ImageMipmap& Image::GetMipmap(size_t id) const
{
    return mMipmaps[id];
}

int Image::GenerateMipmaps(uint32 num)
{
    //empty image
    if (GetData() == 0)
    {
        LOG_WARNING("Tried to generate mMipmaps of an empty image");
        return 0;
    }

    if (mFormat == ImageFormat::BC1 || mFormat == ImageFormat::BC2 || mFormat == ImageFormat::BC3 ||
            mFormat == ImageFormat::BC4 || mFormat == ImageFormat::BC5)
    {
        LOG_WARNING("Block coded (BCx) pixel formats are unsupported");
        return 1;
    }

    if (mFormat == ImageFormat::Unknown)
    {
        LOG_WARNING("Invalid pixel format");
        return 1;
    }

    //start from most detailed mipmap
    ImageMipmap* pMip = &mMipmaps[0];

    for (uint32 i = 0; i < num; i++)
    {
        ImageMipmap nextMip;
        nextMip.width = (pMip->width / 2) + (pMip->width % 2);
        nextMip.height = (pMip->height / 2) + (pMip->height % 2);
        if (nextMip.width == 0) nextMip.width = 1;
        if (nextMip.height == 0) nextMip.height = 1;

        //allocate next mipmap
        nextMip.dataSize = nextMip.width * nextMip.height * BitsPerPixel(mFormat) / 8;
        nextMip.data = malloc(nextMip.dataSize);

        // NEEDS CHANGE - NO SUPPORT FOR non-power-of-2 !!!!
        for (uint32 y = 0; y < nextMip.height; y++)
        {
            for (uint32 x = 0; x < nextMip.width; x++)
            {
                // naive box filter
                Vector a = GetTexel(pMip->data, 2 * x, 2 * y, pMip->width, mFormat);
                Vector b = GetTexel(pMip->data, 2 * x + 1, 2 * y, pMip->width, mFormat);
                Vector c = GetTexel(pMip->data, 2 * x, 2 * y + 1, pMip->width, mFormat);
                Vector d = GetTexel(pMip->data, 2 * x + 1, 2 * y + 1, pMip->width, mFormat);

                SetTexel(((a + b) + (c + d)) * 0.25f, nextMip.data, x, y, nextMip.width, mFormat);
            }
        }

        mMipmaps.push_back(nextMip);

        if (nextMip.width == 1 && nextMip.height == 1)
            break;

        pMip = &mMipmaps[mMipmaps.size() - 1];
    }

    return 0;
}

int Image::Convert(ImageFormat destFormat)
{
    if (destFormat == mFormat)
        return 0;

    //empty image
    if (GetData() == 0)
    {
        LOG_WARNING("Tried to convert pixel format of an empty image");
        return 0;
    }

    // BC conversion - TODO!
    if (mFormat == ImageFormat::BC1 || mFormat == ImageFormat::BC2 || mFormat == ImageFormat::BC3 ||
            mFormat == ImageFormat::BC4 || mFormat == ImageFormat::BC5 ||
            destFormat == ImageFormat::BC1 || destFormat == ImageFormat::BC2 ||
            destFormat == ImageFormat::BC3 ||
            destFormat == ImageFormat::BC4 || destFormat == ImageFormat::BC5)
    {
        //LOG_WARNING("Block coded (BCx) pixel formats are currently impossible to convert"); // TODO
        return 1;
    }


    if (mFormat == ImageFormat::Unknown || destFormat == ImageFormat::Unknown)
    {
        LOG_WARNING("Invalid pixel format");
        return 1;
    }

    for (uint32 i = 0; i < mMipmaps.size(); i++)
    {
        //alloc buffer for new format
        size_t newDataSize = mMipmaps[i].width * mMipmaps[i].height * BitsPerPixel(destFormat) / 8;
        void* pNewData = malloc(newDataSize);

        void* pOldData = mMipmaps[i].data;

        uint32 width = mMipmaps[i].height;
        uint32 height = mMipmaps[i].height;
        Vector tmp;

        for (uint32 y = 0; y < height; y++)
        {
            for (uint32 x = 0; x < width; x++)
            {
                tmp = GetTexel(pOldData, x, y, width, mFormat);
                SetTexel(tmp, pNewData, x, y, width, destFormat);
            }
        }

        free(mMipmaps[i].data);
        mMipmaps[i].data = pNewData;
        mMipmaps[i].dataSize = newDataSize;
    }

    mFormat = destFormat;
    return 0;

    /*
    if ((mFormat == ImageFormat::A_UByte) && (destFormat == ImageFormat::RGBA_UByte))
    {
        mMipmaps[0].dataSize = width * height * 4;
        uchar* pNewData = (uchar*)malloc(mMipmaps[0].dataSize);
        const uchar* pOldData = (uchar*)mMipmaps[0].data;

        for (int y = 0; y<height; y++)
        {
            for (int x = 0; x<width; x++)
            {
                pNewData[4*(y*width+x)  ] = pOldData[y*width+x];
                pNewData[4*(y*width+x)+1] = pOldData[y*width+x];
                pNewData[4*(y*width+x)+2] = pOldData[y*width+x];
                pNewData[4*(y*width+x)+3] = 0xFF;
            }
        }

        free(mMipmaps[0].data);
        mMipmaps[0].data = pNewData;
        mFormat = destFormat;
    }

    if ((mFormat == ImageFormat::RGB_UByte) && (destFormat == ImageFormat::RGBA_UByte))
    {
        mMipmaps[0].dataSize = width * height * 4;
        uchar* pNewData = (uchar*)malloc(mMipmaps[0].dataSize);
        const uchar* pOldData = (uchar*)mMipmaps[0].data;

        for (int y = 0; y<height; y++)
        {
            for (int x = 0; x<width; x++)
            {
                pNewData[4*(y*width+x)  ] = pOldData[3*(y*width+x)  ];
                pNewData[4*(y*width+x)+1] = pOldData[3*(y*width+x)+1];
                pNewData[4*(y*width+x)+2] = pOldData[3*(y*width+x)+2];
                pNewData[4*(y*width+x)+3] = 0xFF;
            }
        }

        free(mMipmaps[0].data);
        mMipmaps[0].data = pNewData;
        mFormat = destFormat;
    }
    */
}

} //namespace Common
} // namespace NFE
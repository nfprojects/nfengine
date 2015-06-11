/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Image function definitions.
 */

#include "../PCH.hpp"
#include "Image.hpp"
#include "../Logger.hpp"
#include "MipmapFilter.hpp"
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
    for (size_t i = 0; i < other.mMipmaps.size(); i++)
    {
        Mipmap mipmap(other.mMipmaps[i]);
        mMipmaps.push_back(mipmap);
    }
}

Image::~Image()
{
    Release();
}

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

void Image::Release()
{
    if (!mMipmaps.empty())
    {
        for (uint32 i = 0; i < mMipmaps.size(); i++)
            delete[] mMipmaps[i].GetData();

        mMipmaps.clear();
    }

    mWidth = 0;
    mHeight = 0;
}

int Image::SetData(uchar* pData, int width, int height, ImageFormat format)
{
    Release();

    Mipmap mipmap;
    mipmap.SetWidth(width);
    mipmap.SetHeight(height);
    mipmap.SetDataSize(width * height * BitsPerPixel(format) / 8);
    mipmap.AllocData();

    if (!mipmap.GetData())
        return 1;

    mWidth = width;
    mHeight = height;
    mFormat = format;
    void* mipmapData = new (std::nothrow) uchar[mipmap.GetDataSize()];
    memcpy(mipmapData, pData, mipmap.GetDataSize());
    mipmap.SetData(static_cast<uchar*>(mipmapData), mipmap.GetDataSize());
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
    if (!LoadJPEG(pStream))
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

const uchar* Image::GetData(uint32 mipmapId) const
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
        return nullptr;

    if (id < mMipmaps.size())
        return &mMipmaps[id];

    return nullptr;
}

int Image::GenerateMipmaps(int filterType, uint32 num)
{
    //empty image
    if (!GetData())
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
    Mipmap* pMip = &mMipmaps[0];

    for (uint32 i = 0; i < num; i++)
    {
        Mipmap nextMip;
        nextMip.SetWidth((pMip->GetWidth() / 2) + (pMip->GetWidth() % 2));
        nextMip.SetHeight((pMip->GetHeight() / 2) + (pMip->GetHeight() % 2));
        if (nextMip.GetWidth() == 0)
            nextMip.SetWidth(1);
        if (nextMip.GetHeight() == 0)
            nextMip.SetHeight(1);

        //allocate next mipmap
        nextMip.SetDataSize(nextMip.GetWidth() * nextMip.GetHeight() * BitsPerPixel(mFormat) / 8);
        nextMip.AllocData();

        // NEEDS CHANGE - NO SUPPORT FOR non-power-of-2 !!!!
        uint32 nextMipHeight = nextMip.GetHeight();
        uint32 nextMipWidth = nextMip.GetWidth();

        MipmapFilter* filter = MipmapFilter::CreateFilter(filterType, pMip);
        for (uint32 y = 0; y < nextMipHeight; y++)
        {
            for (uint32 x = 0; x < nextMipWidth; x++)
            {
                Vector outputTexel = filter->Calc(x, y);
                nextMip.SetTexel(outputTexel, x, y, mFormat);
            }
        }
        //IMPORTANT!
        delete filter;
        mMipmaps.push_back(nextMip);

        if (nextMipWidth == 1 && nextMipHeight == 1)
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

    //uint32 width = GetHeight();
    //uint32 height = GetWidth();
    BufferInputStream* imageToConvert = new (std::nothrow) BufferInputStream(GetData(0), GetMipmap(0)->GetDataSize());
    ImageConverter converter(imageToConvert);
    if (!converter.Convert(destFormat))
    {
        LOG_WARNING("Conversion failed.");
        return 1;
    }
    /*
    void* mipmapData = new (std::nothrow) uchar[mipmap.GetDataSize()];
    if (pStream->Read(mipmap.GetDataSize(), mipmapData) != mipmap.GetDataSize())
    {
        return 1;
    }
    mipmap.SetData(static_cast<uchar*>(mipmapData), mipmap.GetDataSize());
    */
    uchar* convertedData = const_cast<uchar*>(static_cast<const uchar*>(converter.GetOutputData()->GetData()));
    if (SetData(convertedData, mWidth, mHeight, destFormat))
    {
        LOG_WARNING("SetData() function failed.");
        return 1;
    }
    delete imageToConvert;
    return 0;

/*
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
        //alloc buffer for new (std::nothrow) format
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

    / *

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

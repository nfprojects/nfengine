#include "stdafx.hpp"
#include "Image.hpp"
#include "Math/Math.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "jpeg/jpgd.h"
#include "Logger.hpp"

namespace NFE {
namespace Common {

using namespace Math;

const char* Image::FormatToStr(ImageFormat format)
{
    switch (format)
    {
        case ImageFormat::A_UBYTE:
            return "A_uchar";
        case ImageFormat::R_UBYTE:
            return "R_uchar";
        case ImageFormat::RGB_UBYTE:
            return "RGB_uchar";
        case ImageFormat::RGBA_UBYTE:
            return "RGBA_uchar";

        case ImageFormat::R_FLOAT:
            return "R_FLOAT";
        case ImageFormat::RGBA_FLOAT:
            return "RGBA_FLOAT";

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
        case ImageFormat::A_UBYTE:
        case ImageFormat::R_UBYTE:
            return 8;

        case ImageFormat::RGB_UBYTE:
            return 3 * 8;
        case ImageFormat::RGBA_UBYTE:
            return 4 * 8;

        case ImageFormat::R_FLOAT:
            return sizeof(float) * 8;
        case ImageFormat::RGBA_FLOAT:
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

Image::Image()
{
    mWidth = 0;
    mHeight = 0;
}

Image::Image(const Image& src)
{
    mWidth = src.mWidth;
    mHeight = src.mHeight;

    //clone all mMipmaps
    for (uint32 i = 0; i < src.mMipmaps.size(); i++)
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

// get texel from raw data buffer, knowing pixel format and image width
__forceinline Vector XImage_GetTexel(const void* pData, uint32 x, uint32 y, uint32 width,
                                     ImageFormat fmt)
{
    switch (fmt)
    {
        case ImageFormat::A_UBYTE:
        {
            const uchar* pSrc = (const uchar*)pData;
            uchar a = pSrc[y * width + x];
            return Vector(255.0f, 255.0f, 255.0f, (float)a) * g_Byte2Float;
        }

        case ImageFormat::R_UBYTE:
        {
            const uchar* pSrc = (const uchar*)pData;
            uchar r = pSrc[y * width + x];
            return Vector((float)r, 0.0f, 0.0f, 255.0f) * g_Byte2Float;
        }

        case ImageFormat::RGB_UBYTE:
        {
            const uchar* pSrc = (const uchar*)pData;
            uchar r = pSrc[3 * (y * width + x)];
            uchar g = pSrc[3 * (y * width + x) + 1];
            uchar b = pSrc[3 * (y * width + x) + 2];
            return Vector((float)r, (float)g, (float)b, 255.0f) * g_Byte2Float;
        }

        case ImageFormat::RGBA_UBYTE:
        {
            const uchar* pSrc = (const uchar*)pData;
            uchar r = pSrc[4 * (y * width + x)];
            uchar g = pSrc[4 * (y * width + x) + 1];
            uchar b = pSrc[4 * (y * width + x) + 2];
            uchar a = pSrc[4 * (y * width + x) + 3];
            return Vector((float)r, (float)g, (float)b, float(a)) * g_Byte2Float;
        }

        case ImageFormat::R_FLOAT:
        {
            const float* pSrc = (const float*)pData;
            float r = pSrc[y * width + x];
            return Vector(r, 0.0f, 0.0f, 1.0f);
        }

        case ImageFormat::RGBA_FLOAT:
        {
            const float* pSrc = (const float*)pData;
            return Vector(pSrc + 4 * (y * width + x));
        }
    }

    return Vector();
}

// get texel of the raw data buffer, knowing pixel format and image width
__forceinline void XImage_SetTexel(const Vector& v, void* pData, uint32 x, uint32 y, uint32 width,
                                   ImageFormat fmt)
{
    switch (fmt)
    {
        case ImageFormat::A_UBYTE:
        {
            uchar* pSrc = (uchar*)pData;
            pSrc[y * width + x] = (uchar)(v.f[3] * 255.0f);
            break;
        }

        case ImageFormat::R_UBYTE:
        {
            uchar* pSrc = (uchar*)pData;
            pSrc[y * width + x] = (uchar)(v.f[0] * 255.0f);
            break;
        }

        case ImageFormat::RGB_UBYTE:
        {
            uchar* pSrc = (uchar*)pData;
            pSrc[3 * (y * width + x)] = (uchar)(v.f[0] * 255.0f);
            pSrc[3 * (y * width + x) + 1] = (uchar)(v.f[1] * 255.0f);
            pSrc[3 * (y * width + x) + 2] = (uchar)(v.f[2] * 255.0f);
            break;
        }

        case ImageFormat::RGBA_UBYTE:
        {
            uchar* pSrc = (uchar*)pData;
            pSrc[4 * (y * width + x)] = (uchar)(v.f[0] * 255.0f);
            pSrc[4 * (y * width + x) + 1] = (uchar)(v.f[1] * 255.0f);
            pSrc[4 * (y * width + x) + 2] = (uchar)(v.f[2] * 255.0f);
            pSrc[4 * (y * width + x) + 3] = (uchar)(v.f[3] * 255.0f);
            break;
        }

        case ImageFormat::R_FLOAT:
        {
            float* pSrc = (float*)pData;
            pSrc[y * width + x] = v.f[0];
            break;
        }

        case ImageFormat::RGBA_FLOAT:
        {
            Float4* pSrc = (Float4*)pData;
            VectorStore(v, pSrc + y * width + x);
            break;
        }
    }
}

int Image::GenerateMipmaps(uint32 num)
{
    //empty imaga
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

    if (mFormat == ImageFormat::UNKNOWN)
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
                Vector a = XImage_GetTexel(pMip->data, 2 * x, 2 * y, pMip->width, mFormat);
                Vector b = XImage_GetTexel(pMip->data, 2 * x + 1, 2 * y, pMip->width, mFormat);
                Vector c = XImage_GetTexel(pMip->data, 2 * x, 2 * y + 1, pMip->width, mFormat);
                Vector d = XImage_GetTexel(pMip->data, 2 * x + 1, 2 * y + 1, pMip->width, mFormat);

                XImage_SetTexel(((a + b) + (c + d)) * 0.25f, nextMip.data, x, y, nextMip.width, mFormat);
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

    //empty imaga
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


    if (mFormat == ImageFormat::UNKNOWN || destFormat == ImageFormat::UNKNOWN)
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
                tmp = XImage_GetTexel(pOldData, x, y, width, mFormat);
                XImage_SetTexel(tmp, pNewData, x, y, width, destFormat);
            }
        }

        free(mMipmaps[i].data);
        mMipmaps[i].data = pNewData;
        mMipmaps[i].dataSize = newDataSize;
    }

    mFormat = destFormat;
    return 0;

    /*
    if ((mFormat == ImageFormat::A_UBYTE) && (destFormat == ImageFormat::RGBA_UBYTE))
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

    if ((mFormat == ImageFormat::RGB_UBYTE) && (destFormat == ImageFormat::RGBA_UBYTE))
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


int Image::LoadBMP(InputStream* pStream)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    //buffer too small
    if (pStream->GetSize() < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
        return 1;

    //read file header
    pStream->Read(sizeof(BITMAPFILEHEADER), &fileHeader);

    if (fileHeader.bfType != 0x4D42) //signature mismatch
        return 1;
    if (fileHeader.bfSize > pStream->GetSize()) //wrong file size
        return 1;

    //read info header
    //memcpy(&infoHeader, pBuffer+offset, sizeof(BITMAPINFOHEADER));
    //offset += sizeof(BITMAPINFOHEADER);
    pStream->Read(sizeof(BITMAPINFOHEADER), &infoHeader);

    //Size of one line in bytes. For BMP it must be multiple of 4
    int lineSize;


    pStream->Seek(fileHeader.bfOffBits);
    size_t offset = fileHeader.bfOffBits;

    mWidth = infoHeader.biWidth;
    mHeight = infoHeader.biWidth;

    if (infoHeader.biBitCount == 24)
    {
        mFormat = ImageFormat::RGBA_UBYTE;
        uchar* pImageData = (uchar*)malloc(mWidth * mHeight * 4);

        if (pImageData == 0)
        {
            Release();
            return 1;
        }

        lineSize = mWidth * 3;
        while (lineSize % 4)
            lineSize++;

        for (int y = 0; y < mHeight; y++)
        {
            for (int x = 0; x < mWidth; x++)
            {
                uchar tmp[3];
                pStream->Read(3, tmp);

                pImageData[4 * (y * mWidth + x)  ] = tmp[0]; //((uchar*)pData)[offset + 3*x + 2];
                pImageData[4 * (y * mWidth + x) + 1] = tmp[1]; //((uchar*)pData)[offset + 3*x + 1];
                pImageData[4 * (y * mWidth + x) + 2] = tmp[2]; // ((uchar*)pData)[offset + 3*x    ];
                pImageData[4 * (y * mWidth + x) + 3] = 255;
            }
            offset += lineSize;
            pStream->Seek(offset);
        }

        ImageMipmap mipmap;
        mipmap.data = pImageData;
        mipmap.dataSize = mWidth * mHeight * 4;
        mipmap.width = mWidth;
        mipmap.height = mHeight;
        mMipmaps.push_back(mipmap);
    }
    else
    {
        Release();
        return 1;
    }

    return 0;
}


// bridge between InputStream and jpgd::decompress_jpeg_image_from_stream function
class CustomJpegStream : public jpgd::jpeg_decoder_stream
{
private:
    InputStream* mStream;

public:
    CustomJpegStream(InputStream* pStream)
    {
        mStream = pStream;
    }

    int read(jpgd::uint8* pBuf, int max_bytes_to_read, bool* pEOF_flag)
    {
        size_t bytesRead = mStream->Read(max_bytes_to_read, pBuf);
        if (bytesRead == 0)
            *pEOF_flag = true;

        return static_cast<int>(bytesRead);
    }
};

int Image::LoadJPEG(InputStream* pStream)
{
    CustomJpegStream jpegStream(pStream);

    int comps;
    ImageMipmap mipmap;
    mipmap.data = jpgd::decompress_jpeg_image_from_stream(&jpegStream, &mWidth, &mHeight, &comps,
                  4);
    mipmap.dataSize = mWidth * mHeight * 4;
    mipmap.width = mWidth;
    mipmap.height = mHeight;

    if (mipmap.data)
    {
        mFormat = ImageFormat::RGBA_UBYTE;
        mMipmaps.push_back(mipmap);
        return 0;
    }
    else
        return 1;
}



// ---------------------------------------------------------------------------
// PNG
// ---------------------------------------------------------------------------

#include "libpng/png.h"

#define PNGSIGSIZE 8


struct pngReadInfo
{
    char* pData;
    size_t offset;
};

void userReadData(png_structp pngPtr, png_bytep data, png_size_t length)
{
    //Here we get our IO pointer back from the read struct.
    //This is the parameter we passed to the png_set_read_fn() function.
    png_voidp a = png_get_io_ptr(pngPtr);
    pngReadInfo* pInfo = (pngReadInfo*)a;

    InputStream* pStream = (InputStream*)(pInfo->pData);

    if (pStream)
        pInfo->offset += pStream->Read(length, data);

    //memcpy(data, (pInfo->pData)+(pInfo->offset), length);
    //pInfo->offset += length;
}

int Image::LoadPNG(InputStream* pStream)
{
    ImageMipmap mipmap;
    mipmap.data = 0;

    //read png signature
    uchar signature[PNGSIGSIZE];
    if (pStream->Read(PNGSIGSIZE, signature) != PNGSIGSIZE)
        return 1;

    //verify png signature
    int is_png = png_sig_cmp((png_byte*)signature, 0, PNGSIGSIZE);
    if (is_png != 0)
        return 1;

    //reading struct
    png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!pngPtr)
        return 1;

    //info struct
    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        png_destroy_read_struct(&pngPtr, (png_infopp)0, (png_infopp)0);
        return 1;
    }


    png_bytep* rowPtrs = nullptr;
    if (setjmp(png_jmpbuf(pngPtr)))
    {
        //An error occured, so clean up what we have allocated so far...
        png_destroy_read_struct(&pngPtr, &infoPtr, (png_infopp)0);
        if (rowPtrs != nullptr) delete [] rowPtrs;
        if (mipmap.data != nullptr) free(mipmap.data);

        return 1;
    }


    //set up reading callback
    pngReadInfo readInfo;
    readInfo.offset = PNGSIGSIZE;
    readInfo.pData = (char*)pStream;
    png_set_read_fn(pngPtr, (png_voidp)&readInfo, userReadData);


    //Set the amount signature bytes we've already read:
    //We've defined PNGSIGSIZE as 8;
    png_set_sig_bytes(pngPtr, PNGSIGSIZE);

    //Now call png_read_info with our pngPtr as image handle, and infoPtr to receive the file info.
    png_read_info(pngPtr, infoPtr);

    png_uint_32 imgWidth =  png_get_image_width(pngPtr, infoPtr);
    png_uint_32 imgHeight = png_get_image_height(pngPtr, infoPtr);
    mWidth = imgWidth;
    mHeight = imgHeight;


    //bits per CHANNEL! note: not per pixel!
    png_uint_32 bitdepth   = png_get_bit_depth(pngPtr, infoPtr);

    //Number of channels
    png_uint_32 channels   = png_get_channels(pngPtr, infoPtr);

    //Color type. (RGB, RGBA, Luminance, luminance alpha... palette... etc)
    png_uint_32 color_type = png_get_color_type(pngPtr, infoPtr);

    switch (color_type)
    {
        case PNG_COLOR_TYPE_PALETTE:
            png_set_palette_to_rgb(pngPtr);
            //Don't forget to update the channel info (thanks Tom!)
            //It's used later to know how big a buffer we need for the image
            channels = 3;
            mFormat = ImageFormat::RGB_UBYTE;
            break;

        case PNG_COLOR_TYPE_GRAY:
            if (bitdepth < 8)
                png_set_expand_gray_1_2_4_to_8(pngPtr);
            //And the bitdepth info
            bitdepth = 8;
            mFormat = ImageFormat::A_UBYTE;
            break;

        case PNG_COLOR_TYPE_RGB:
            mFormat = ImageFormat::RGB_UBYTE;
            break;

        case PNG_COLOR_TYPE_RGBA:
            mFormat = ImageFormat::RGBA_UBYTE;
            break;
    }

    /*if the image has a transperancy set.. convert it to a full Alpha channel..*/
    if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(pngPtr);
        channels++;
    }


    //Here's one of the pointers we've defined in the error handler section:
    //Array of row pointers. One for every row.
    rowPtrs = new png_bytep [imgHeight];



    mipmap.dataSize = imgWidth * imgHeight * bitdepth * channels / 8;
    mipmap.data = malloc(mipmap.dataSize);
    mipmap.width = mWidth;
    mipmap.height = mHeight;


    //This is the length in bytes, of one row.
    const unsigned int stride = imgWidth * bitdepth * channels / 8;

    //A little for-loop here to set all the row pointers to the starting
    //Adresses for every row in the buffer
    for (png_uint_32 i = 0; i < imgHeight; i++)
    {
        //Set the pointer to the data pointer + i times the row stride.
        //Notice that the row order is reversed with q.
        //This is how at least OpenGL expects it,
        //and how many other image loaders present the data.
        png_uint_32 q = i * stride;
        rowPtrs[i] = (png_bytep)mipmap.data + q;
    }

    //And here it is! The actuall reading of the image!
    //Read the imagedata and write it to the adresses pointed to
    //by rowptrs (in other words: our image databuffer)
    png_read_image(pngPtr, rowPtrs);



    delete[] (png_bytep)rowPtrs;
    png_destroy_read_struct(&pngPtr, &infoPtr, (png_infopp)0);

    mMipmaps.push_back(mipmap);
    return 0;
}


// ---------------------------------------------------------------------------
// DDS
// ---------------------------------------------------------------------------

struct DDS_PIXELFORMAT
{
    unsigned int    dwSize;
    unsigned int    dwFlags;
    unsigned int    dwFourCC;
    unsigned int    dwRGBBitCount;
    unsigned int    dwRBitMask;
    unsigned int    dwGBitMask;
    unsigned int    dwBBitMask;
    unsigned int    dwAlphaBitMask;
};

struct DDS_header
{
    unsigned int    dwMagic;
    unsigned int    dwSize;
    unsigned int    dwFlags;
    unsigned int    dwHeight;
    unsigned int    dwWidth;
    unsigned int    dwPitchOrLinearSize;
    unsigned int    dwDepth;
    unsigned int    dwMipMapCount;
    unsigned int    dwReserved1[ 11 ];

    //  DDPIXELFORMAT
    DDS_PIXELFORMAT sPixelFormat;

    //  DDCAPS2
    struct
    {
        unsigned int    dwCaps1;
        unsigned int    dwCaps2;
        unsigned int    dwDDSX;
        unsigned int    dwReserved;
    } sCaps;

    unsigned int dwReserved2;
};

#define DDS_MAGIC_NUMBER    0x20534444

#define DDSD_CAPS   0x1
#define DDSD_HEIGHT 0x2
#define DDSD_WIDTH  0x4
#define DDSD_PITCH  0x8
#define DDSD_PIXELFORMAT    0x1000
#define DDSD_MIPMAPCOUNT    0x20000
#define DDSD_LINEARSIZE 0x80000
#define DDSD_DEPTH  0x800000

#define DDPF_ALPHAPIXELS 0x00000001
#define DDPF_ALPHA 0x00000002
#define DDPF_FOURCC 0x00000004
#define DDPF_PALETTEINDEXED8 0x00000020
#define DDPF_RGB 0x00000040
#define DDPF_LUMINANCE 0x00020000

#define ID_DXT1   0x31545844
#define ID_DXT3   0x33545844
#define ID_DXT5   0x35545844

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
    ((uint32)(uchar)(ch0) | ((uint32)(uchar)(ch1) << 8) |       \
     ((uint32)(uchar)(ch2) << 16) | ((uint32)(uchar)(ch3) << 24))
#endif /* defined(MAKEFOURCC) */

#define ISBITMASK(r, g, b, a) (ddpf.dwRBitMask == r && ddpf.dwGBitMask == g && ddpf.dwBBitMask == b && ddpf.dwAlphaBitMask == a)

static ImageFormat XImage_DDSGetFormat(const DDS_PIXELFORMAT& ddpf)
{
    if (ddpf.dwFlags & DDPF_RGB)
    {
        // Note that sRGB formats are written using the "DX10" extended header

        switch (ddpf.dwRGBBitCount)
        {
            case 32:
                if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
                    return ImageFormat::RGBA_UBYTE; //DXGI_FORMAT_R8G8B8A8_UNORM;

                if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
                    // Only 32-bit color channel format in D3D9 was R32F
                    return ImageFormat::R_FLOAT; //DXGI_FORMAT_R32_FLOAT

                //if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
                //  return DXGI_FORMAT_B8G8R8A8_UNORM;

                //if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
                //    return DXGI_FORMAT_B8G8R8X8_UNORM;

                // No DXGI format maps to ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000) aka D3DFMT_X8B8G8R8

                // Note that many common DDS reader/writers (including D3DX) swap the
                // the RED/BLUE masks for 10:10:10:2 formats. We assumme
                // below that the 'backwards' header mask is being used since it is most
                // likely written by D3DX. The more robust solution is to use the 'DX10'
                // header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

                // For 'correct' writers, this should be 0x000003ff, 0x000ffc00, 0x3ff00000 for RGB data
                //if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
                //    return DXGI_FORMAT_R10G10B10A2_UNORM;

                // No DXGI format maps to ISBITMASK(0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000) aka D3DFMT_A2R10G10B10
                //if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
                //    return DXGI_FORMAT_R16G16_UNORM;

                break;

            case 24:
                // No 24bpp DXGI formats aka D3DFMT_R8G8B8
                break;

            case 16:
                /*
                if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
                    return DXGI_FORMAT_B5G5R5A1_UNORM;

                if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
                    return DXGI_FORMAT_B5G6R5_UNORM;

                if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
                    return DXGI_FORMAT_B4G4R4A4_UNORM;
                    */

                break;
        }
    }
    else if (ddpf.dwFlags & DDPF_LUMINANCE)
    {
        if (8 == ddpf.dwRGBBitCount)
        {
            if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
                return ImageFormat::A_UBYTE; //DXGI_FORMAT_R8_UNORM
        }

        /*
        if (16 == ddpf.RGBBitCount)
        {
            if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
                return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension

            if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
                return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
        }
        */
    }
    else if (ddpf.dwFlags & DDPF_ALPHA)
    {
        if (8 == ddpf.dwRGBBitCount)
            return ImageFormat::A_UBYTE; //DXGI_FORMAT_A8_UNORM
    }
    else if (ddpf.dwFlags & DDPF_FOURCC)
    {
        if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.dwFourCC)
            return ImageFormat::BC1; //DXGI_FORMAT_BC1_UNORM

        if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.dwFourCC)
            return ImageFormat::BC1; //DXGI_FORMAT_BC2_UNORM

        if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.dwFourCC)
            return ImageFormat::BC3; //DXGI_FORMAT_BC3_UNORM


        // While pre-mulitplied alpha isn't directly supported by the DXGI formats,
        // they are basically the same as these BC formats so they can be mapped
        if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.dwFourCC)
            return ImageFormat::BC2; //DXGI_FORMAT_BC2_UNORM

        if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.dwFourCC)
            return ImageFormat::BC3; //DXGI_FORMAT_BC3_UNORM

        if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.dwFourCC)
            return ImageFormat::BC4; //DXGI_FORMAT_BC4_UNORM

        if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.dwFourCC)
            return ImageFormat::BC4; //DXGI_FORMAT_BC4_UNORM

        if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.dwFourCC)
            return ImageFormat::BC4; //DXGI_FORMAT_BC4_SNORM

        if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.dwFourCC)
            return ImageFormat::BC5; //DXGI_FORMAT_BC5_UNORM

        if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.dwFourCC)
            return ImageFormat::BC5; //DXGI_FORMAT_BC5_UNORM

        if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.dwFourCC)
            return ImageFormat::BC5; //DXGI_FORMAT_BC1_SNORM


        // Check for D3DFORMAT enums being set here
        switch (ddpf.dwFourCC)
        {
            /*
            case 36: // D3DFMT_A16B16G16R16
                return DXGI_FORMAT_R16G16B16A16_UNORM;

            case 110: // D3DFMT_Q16W16V16U16
                return DXGI_FORMAT_R16G16B16A16_SNORM;

            case 111: // D3DFMT_R16F
                return DXGI_FORMAT_R16_FLOAT;

            case 112: // D3DFMT_G16R16F
                return DXGI_FORMAT_R16G16_FLOAT;

            case 113: // D3DFMT_A16B16G16R16F
                return DXGI_FORMAT_R16G16B16A16_FLOAT;
                */

            case 114: // D3DFMT_R32F
                return ImageFormat::R_FLOAT; //DXGI_FORMAT_R32_FLOAT

            //case 115: // D3DFMT_G32R32F
            //  return DXGI_FORMAT_R32G32_FLOAT;

            case 116: // D3DFMT_A32B32G32R32F
                return ImageFormat::RGBA_FLOAT; //DXGI_FORMAT_R32G32B32A32_FLOAT
        }
    }

    return ImageFormat::UNKNOWN;
}

int Image::LoadDDS(InputStream* pStream)
{
    // read header
    DDS_header header;
    if (pStream->Read(sizeof(header), &header) != sizeof(header))
        return 1;

    //check magic number
    if (header.dwMagic != DDS_MAGIC_NUMBER)
        return 1;

    mWidth = header.dwWidth;
    mHeight = header.dwHeight;

    int numMipmaps = header.dwMipMapCount;
    if (numMipmaps < 1) numMipmaps = 1;
    if (numMipmaps > 32) return 1;


    mFormat = XImage_DDSGetFormat(header.sPixelFormat);

    if (mFormat == ImageFormat::UNKNOWN)
        return 1;

    int width = mWidth;
    int height = mHeight;
    for (int i = 0; i < numMipmaps; i++)
    {
        //keep mipmap size > 0
        if (width == 0) width = 1;
        if (height == 0) height = 1;

        ImageMipmap mipmap;
        mipmap.width = width;
        mipmap.height = height;
        mipmap.dataSize = ((width + 3) / 4) * ((height + 3) / 4) * 16 * BitsPerPixel(mFormat) / 8;
        mipmap.data = malloc(mipmap.dataSize);
        mMipmaps.push_back(mipmap);

        if (pStream->Read(mipmap.dataSize, mipmap.data) != mipmap.dataSize)
        {
            return 1;
        }

        width /= 2;
        height /= 2;
    }

    return 0;
}

} //namespace Common
} // namespace NFE
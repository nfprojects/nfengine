/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Image function definitions.
 */

#include "../PCH.hpp"
#include "Image.hpp"
#include "../Math/Math.hpp"
#include "libpng/png.h"


#define PNGSIGSIZE 8

namespace NFE {
namespace Common {

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
    pngReadInfo* pInfo = static_cast<pngReadInfo*>(a);

    InputStream* pStream = static_cast<InputStream*>(pInfo->pData);

    if (pStream)
        pInfo->offset += pStream->Read(length, data);

    //memcpy(data, (pInfo->pData)+(pInfo->offset), length);
    //pInfo->offset += length;
}

int Image::LoadPNG(InputStream* pStream)
{
    pngInfoPPZero = static_cast<png_infopp>(0);
    //read png signature
    uchar signature[PNGSIGSIZE];
    if (pStream->Read(PNGSIGSIZE, signature) != PNGSIGSIZE)
        return 1;

    //verify png signature
    int is_png = png_sig_cmp(static_cast<png_byte*>(signature), 0, PNGSIGSIZE);
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
        png_destroy_read_struct(&pngPtr, pngInfoPPZero, pngInfoPPZero);
        return 1;
    }


    png_bytep* rowPtrs = nullptr;
    if (setjmp(png_jmpbuf(pngPtr)))
    {
        //An error occured, so clean up what we have allocated so far...
        png_destroy_read_struct(&pngPtr, &infoPtr, pngInfoPPZero);
        if (rowPtrs != nullptr) delete [] rowPtrs;
        //if (mipmap.GetData()) delete mipmap.data;

        return 1;
    }


    //set up reading callback
    pngReadInfo readInfo;
    readInfo.offset = PNGSIGSIZE;
    readInfo.pData = static_cast<char*>(pStream);
    png_set_read_fn(pngPtr, static_cast<png_voidp>(&readInfo), userReadData);


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
            mFormat = ImageFormat::RGB_UByte;
            break;

        case PNG_COLOR_TYPE_GRAY:
            if (bitdepth < 8)
                png_set_expand_gray_1_2_4_to_8(pngPtr);
            //And the bitdepth info
            bitdepth = 8;
            mFormat = ImageFormat::A_UByte;
            break;

        case PNG_COLOR_TYPE_RGB:
            mFormat = ImageFormat::RGB_UByte;
            break;

        case PNG_COLOR_TYPE_RGBA:
            mFormat = ImageFormat::RGBA_UByte;
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


    Mipmap mipmap;
    mipmap.SetDataSize(imgWidth * imgHeight * bitdepth * channels / 8);
    mipmap.Alloc();
    mipmap.SetWidth(mWidth);
    mipmap.SetHeight(mHeight);


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
        rowPtrs[i] = static_cast<png_bytep>(mipmap.GetData() + q);
    }

    //And here it is! The actuall reading of the image!
    //Read the imagedata and write it to the adresses pointed to
    //by rowptrs (in other words: our image databuffer)
    png_read_image(pngPtr, rowPtrs);



    delete[] rowPtrs;
    png_destroy_read_struct(&pngPtr, &infoPtr, pngInfoPPZero);

    mMipmaps.push_back(mipmap);
    return 0;
}

} // namespace Common
} // namespace NFE
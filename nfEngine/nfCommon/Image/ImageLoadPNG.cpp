/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  LoadPNG function definition from Image class.
 */

#include "../PCH.hpp"
#include "Image.hpp"
#include "libpng/png.h"
#include "../Logger.hpp"

#define PNGSIGSIZE 8

namespace NFE {
namespace Common {

struct pngReadInfo
{
    char* data;
    size_t offset;
};

void userReadData(png_structp pngPtr, png_bytep data, png_size_t length)
{
    png_voidp a = png_get_io_ptr(pngPtr);
    pngReadInfo* info = static_cast<pngReadInfo*>(a);
    void* infoData = info->data;
    InputStream* stream = static_cast<InputStream*>(infoData);

    if (stream)
        info->offset += stream->Read(length, data);
}

bool Image::LoadPNG(InputStream* stream)
{
    png_infopp pngInfoPPZero = static_cast<png_infopp>(0);

    // read png signature
    uchar signature[PNGSIGSIZE];
    if (stream->Read(PNGSIGSIZE, signature) != PNGSIGSIZE)
    {
        LOG_ERROR("Reading PNG signature failed.");
        return false;
    }

    // verify png signature
    int is_png = png_sig_cmp(static_cast<png_byte*>(signature), 0, PNGSIGSIZE);
    if (is_png != 0)
    {
        LOG_ERROR("Veryfing PNG signature failed.");
        return false;
    }

    // reading struct
    png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!pngPtr)
    {
        LOG_ERROR("Reading PNG structure failed.");
        return false;
    }

    // info struct
    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        LOG_ERROR("Creating PNG information structure failed.");
        png_destroy_read_struct(&pngPtr, pngInfoPPZero, pngInfoPPZero);
        return false;
    }

    if (setjmp(png_jmpbuf(pngPtr)))
    {
        LOG_ERROR("Saving calling environment for long jump in PNG failed.");
        png_destroy_read_struct(&pngPtr, &infoPtr, pngInfoPPZero);
        return false;
    }

    // set up reading callback
    pngReadInfo readInfo;
    readInfo.offset = PNGSIGSIZE;
    readInfo.data = (char*)stream;
    png_set_read_fn(pngPtr, static_cast<png_voidp>(&readInfo), userReadData);

    // Set the amount signature bytes
    png_set_sig_bytes(pngPtr, PNGSIGSIZE);

    // Now call png_read_info to receive the file info.
    png_read_info(pngPtr, infoPtr);

    png_uint_32 imgWidth =  png_get_image_width(pngPtr, infoPtr);
    png_uint_32 imgHeight = png_get_image_height(pngPtr, infoPtr);
    mWidth = imgWidth;
    mHeight = imgHeight;

    png_uint_32 bitdepth   = png_get_bit_depth(pngPtr, infoPtr);
    png_uint_32 channels   = png_get_channels(pngPtr, infoPtr);
    png_uint_32 color_type = png_get_color_type(pngPtr, infoPtr);

    switch (color_type)
    {
        case PNG_COLOR_TYPE_PALETTE:
            png_set_palette_to_rgb(pngPtr);
            channels = 3;
            mFormat = ImageFormat::RGB_UByte;
            break;

        case PNG_COLOR_TYPE_GRAY:
            if (bitdepth < 8)
                png_set_expand_gray_1_2_4_to_8(pngPtr);
            bitdepth = 8;
            mFormat = ImageFormat::A_UByte;
            break;

        case PNG_COLOR_TYPE_RGB:
            mFormat = ImageFormat::RGB_UByte;
            break;

        case PNG_COLOR_TYPE_RGBA:
            mFormat = ImageFormat::RGBA_UByte;
            break;

        default:
            mFormat = ImageFormat::Unknown;
            LOG_ERROR("PNG color type not recognized.");
            return false;
    }

    // if the image has a transperancy set, convert it to a full Alpha channel
    if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(pngPtr);
        channels++;
    }

    size_t dataSize = imgWidth * imgHeight * bitdepth * channels / 8;
    std::unique_ptr<png_bytep[]> rowPtrs(new (std::nothrow) png_bytep[imgHeight]);
    std::unique_ptr<uchar[]> dataPtr(new (std::nothrow) uchar[dataSize]);

    if (!rowPtrs.get() || !dataPtr.get())
    {
        LOG_ERROR("Allocating memory for loading PNG image failed.");
        Release();
        return false;
    }

    // row length in bytes
    const unsigned int stride = imgWidth * bitdepth * channels / 8;

    // A loop to set all row pointers to the starting
    // adresses for every row in the buffer
    for (png_uint_32 i = 0; i < imgHeight; i++)
    {
        png_uint_32 q = i * stride;
        rowPtrs[i] = static_cast<png_bytep>(static_cast<uchar*>(dataPtr.get()) + q);
    }

    // Read the imagedata and write it to the adresses pointed to
    // by rowptrs
    png_read_image(pngPtr, rowPtrs.get());

    // Create mipmap based on data read from PNG
    Mipmap mipmap(dataPtr.get(), imgWidth, imgHeight, dataSize);
    png_destroy_read_struct(&pngPtr, &infoPtr, pngInfoPPZero);

    mMipmaps.push_back(std::move(mipmap));
    return true;
}

} // namespace Common
} // namespace NFE
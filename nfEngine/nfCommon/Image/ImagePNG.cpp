/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImagePNG class definition.
 */

#include "../PCH.hpp"
#include "ImagePNG.hpp"
#include "Image.hpp"
#include "libpng/png.h"
#include "../Logger.hpp"

#define PNGSIGSIZE 8

namespace NFE {
namespace Common {

namespace {

struct pngReadInfo
{
    char* data;
    size_t offset;
};

bool usingTrnsChunk(png_structp pngPtr, png_infop infoPtr)
{
    if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(pngPtr);
        return true;
    }

    return false;
}

void userReadData(png_structp pngPtr, png_bytep data, png_size_t length)
{
    png_voidp a = png_get_io_ptr(pngPtr);
    pngReadInfo* info = static_cast<pngReadInfo*>(a);
    void* infoData = info->data;
    InputStream* stream = static_cast<InputStream*>(infoData);

    if (stream)
        info->offset += stream->Read(data, length);
}

}

// Register PNG image type
bool gImagePNGRegistered = ImageType::RegisterImageType("PNG", std::make_unique<ImagePNG>());

bool ImagePNG::Check(InputStream* stream)
{
    uint32 signature = 0;
    stream->Seek(0);
    if (sizeof(signature) < stream->Read(&signature, sizeof(signature)))
    {
        LOG_ERROR("Could not read signature from the stream.");
        return false;
    }

    stream->Seek(0);

    return signature == 0x474E5089;
}

bool ImagePNG::Load(Image* img, InputStream* stream)
{
    // read png signature
    uint8 signature[PNGSIGSIZE];
    if (stream->Read(signature, PNGSIGSIZE) != PNGSIGSIZE)
    {
        LOG_ERROR("Reading PNG signature failed.");
        return false;
    }

    // verify png signature
    if (!png_check_sig(static_cast<png_const_bytep>(signature), PNGSIGSIZE))
    {
        LOG_ERROR("Veryfing PNG signature failed.");
        return false;
    }

    // reading struct
    png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!pngPtr)
    {
        LOG_ERROR("Creating PNG structure failed.");
        return false;
    }

    // info struct
    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        LOG_ERROR("Creating PNG information structure failed.");
        png_destroy_read_struct(&pngPtr, nullptr, nullptr);
        return false;
    }

    if (setjmp(png_jmpbuf(pngPtr)))
    {
        LOG_ERROR("Saving calling environment for long jump in PNG failed.");
        png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
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

    png_uint_32 bitdepth   = png_get_bit_depth(pngPtr, infoPtr);
    png_uint_32 channels   = png_get_channels(pngPtr, infoPtr);
    png_uint_32 color_type = png_get_color_type(pngPtr, infoPtr);

    // when 16bpp then scale it down to 8bp
    if (bitdepth > 8)
        png_set_scale_16(pngPtr);

    ImageFormat format = ImageFormat::Unknown;

    switch (color_type)
    {
        case PNG_COLOR_TYPE_PALETTE:
            png_set_palette_to_rgb(pngPtr);
            channels = 3;

            format = ImageFormat::RGB_UByte; //can be tRNS

            // if the image has a transperancy set, convert it to a full Alpha channel
            if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
            {
                // if there is tRNS,
                // then png_set_palette_to_rgb(pngPtr) should transform it to rgba
                format = ImageFormat::RGBA_UByte;
                channels = 4;
            }
            break;

        case PNG_COLOR_TYPE_GRAY:

            // if the image has a transperancy set, convert it to a full Alpha channel
            if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
            {
                png_set_gray_to_rgb(pngPtr);
                format = ImageFormat::RGBA_UByte;
                channels = 4;
            }
            else
            {
                png_set_expand_gray_1_2_4_to_8(pngPtr);
                format = ImageFormat::A_UByte;
            }
            break;

        case PNG_COLOR_TYPE_RGB:
            format = ImageFormat::RGB_UByte; //can be tRNS

            // if the image has a transperancy set, convert it to a full Alpha channel
            if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
            {
                png_set_tRNS_to_alpha(pngPtr);
                format = ImageFormat::RGBA_UByte;
                channels = 4;
            }
            break;

        case PNG_COLOR_TYPE_RGBA:
            format = ImageFormat::RGBA_UByte;
            break;

        case PNG_COLOR_TYPE_GA:
            // Change G->RGB or GA->RGBA
            png_set_gray_to_rgb(pngPtr);
            channels = 4;
            format = ImageFormat::RGBA_UByte;
            break;

        default:
            LOG_ERROR("PNG color type %d not recognized.", color_type);
            return false;
    }

    bitdepth = 8;

    size_t dataSize = imgWidth * imgHeight * bitdepth * channels / 8;
    std::unique_ptr<png_bytep[]> rowPtrs(new (std::nothrow) png_bytep[imgHeight]);
    std::unique_ptr<uint8[]> dataPtr(new (std::nothrow) uint8[dataSize]);

    if (!rowPtrs.get() || !dataPtr.get())
    {
        LOG_ERROR("Allocating memory for loading PNG image failed.");
        img->Release();
        return false;
    }

    // row length in bytes
    const unsigned int stride = imgWidth * bitdepth * channels / 8;

    // A loop to set all row pointers to the starting
    // adresses for every row in the buffer
    for (png_uint_32 i = 0; i < imgHeight; i++)
    {
        png_uint_32 q = i * stride;
        rowPtrs[i] = static_cast<png_bytep>(static_cast<uint8*>(dataPtr.get()) + q);
    }

    // Read the imagedata and write it to the adresses pointed to
    // by rowptrs
    png_read_image(pngPtr, rowPtrs.get());

    // Create mipmap based on data read from PNG
    bool result = img->SetData(dataPtr.get(), imgWidth, imgHeight, format);
    png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);

    return result;
}

bool ImagePNG::Save(Image*, OutputStream*)
{
    return false;
}

} // namespace Common
} // namespace NFE
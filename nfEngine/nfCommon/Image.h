/**
    NFEngine project

    \file   Image.h
    \brief  Image class declaration.
*/

#pragma once

#include "nfCommon.h"

namespace NFE {
namespace Common {

/**
 * Image formats
 */
enum class ImageFormat
{
    UNKNOWN,

    A_UBYTE,     // 8 bit
    R_UBYTE,     // 8 bit
    RGB_UBYTE,   // 24 bit
    RGBA_UBYTE,  // 32 bit

    R_FLOAT,     // 32 bit
    RGBA_FLOAT,  // 128 bit

    BC1,         // block coding 1, aka. DXT1 - 4 bits per pixel
    BC2,         // block coding 2, aka. DXT3 - 8 bits per pixel
    BC3,         // block coding 3, aka. DXT5 - 8 bits per pixel
    BC4,         // block coding 4 - 4 bits per pixel
    BC5,         // block coding 5 - 8 bits per pixel
};

class InputStream;
class OutputStream;

/**
 * Helper structure describing single mipmap level.
 */
struct ImageMipmap
{
    void* data;
    size_t dataSize;
    uint32 width;
    uint32 height;
};

class NFCOMMON_API Image
{
    std::vector<ImageMipmap> mMipmaps;
    int mWidth;
    int mHeight;
    ImageFormat mFormat;

    int LoadBMP(InputStream* pStream);
    int LoadPNG(InputStream* pStream);
    int LoadJPEG(InputStream* pStream);
    int LoadDDS(InputStream* pStream);

public:
    /**
     * Convert ImageFormat to string
     */
    static const char* FormatToStr(ImageFormat format);

    /**
     * Retrieve number of bits per pixel for an image format
     */
    static size_t BitsPerPixel(ImageFormat format);

    Image();
    Image(const Image& src);
    ~Image();

    /**
     * Free the image from memory.
     */
    void Release();

    /**
     * Generate mipmaps.
     * @param num Number of mipmaps to generate. Leave this argument default to generate a full set
     * @return 0 on success
     */
    int GenerateMipmaps(uint32 num = 0xFFFFFFFF);

    /**
     * Change pixel format. Currently converting from and to BC formats is unsupported.
     * @param destFormat Destination image format
     * @return 0 on success
     */
    int Convert(ImageFormat destFormat);

    /**
     * Create an image with custom pixels buffer.
     * @param pData Source data buffer
     * @param width Image width
     * @param height Image height
     * @param format Source data format
     * @return 0 on success
     */
    int SetData(void* pData, int width, int height, ImageFormat format);

    /**
     * Load image from a data stream. Supported file formats: BMP, JPEG, PNG, DDS.
     * @param pStream A pointer to a stream containing image file
     * @return 0 on success
     */
    int Load(InputStream* pStream);

    /**
     * Get width of the image (in pixels).
     */
    int GetWidth() const;

    /**
     * Get height of the image (in pixels).
     */
    int GetHeight() const;

    /**
     * Get image content.
     * @param mipmap Mipmap level
     */
    void* GetData(uint32 mipmap = 0) const;

    /**
     * Get format of the image.
     */
    ImageFormat GetFormat() const;

    /**
     * Get number of mipmaps in the image.
     */
    size_t GetMipmapsNum() const;

    /**
     * Access n-th mipmap data.
     * @param id Mipmap level.
     */
    const ImageMipmap& GetMipmap(size_t id) const;
};

} // namespace Common
} // namespace NFE

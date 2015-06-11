/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Image class declaration.
 */

#pragma once

#include "Mipmap.hpp"
//#include "ImageFormat.hpp"
//#include "MipmapFilter.hpp"
//#include "ImageConverter.hpp"
//#include "../nfCommon.hpp"
#include "../InputStream.hpp"
//#include "../OutputStream.hpp"

namespace NFE {
namespace Common {

//class InputStream;
//class OutputStream;

class NFCOMMON_API Image
{
    std::vector<Mipmap> mMipmaps;
    int mWidth;
    int mHeight;
    ImageFormat mFormat;

    int LoadBMP(InputStream* pStream);
    int LoadPNG(InputStream* pStream);
    int LoadJPEG(InputStream* pStream);
    int LoadDDS(InputStream* pStream);

public:
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
    int GenerateMipmaps(int filterType, uint32 num = 0xFFFFFFFF);

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
    int SetData(uchar* pData, int width, int height, ImageFormat format);

    /**
     * Load image from a data stream. Supported file formats: BMP, JPEG, PNG, DDS.
     * @param pStream A pointer to a stream containing image file
     * @return 0 on success
     */
    int Load(InputStream* pStream);

         /**
     * Convert ImageFormat to string
     * @param format Source data format
     */
    static const char* FormatToStr(ImageFormat format);

    /**
     * Retrieve number of bits per pixel for an image format
     * @param format Source data format
     */
    static size_t BitsPerPixel(ImageFormat format);

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
    const uchar* GetData(uint32 mipmap = 0) const;

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
    const Mipmap* Image::GetMipmap(uint32 id) const;
};

} // namespace Common
} // namespace NFE

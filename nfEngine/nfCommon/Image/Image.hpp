/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Image class declaration.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../InputStream.hpp"
#include "Mipmap.hpp"
#include "ImageFormat.hpp"
#include "ImageType.hpp"

#include <unordered_map>
#include <memory>
#include <vector>

namespace NFE {
namespace Common {

// Typedefs to make these types shorter and more readable
//using ImageTypePtr = std::unique_ptr<ImageType>;
//using ImageTypeMap = std::unordered_map<std::string, ImageTypePtr>;

class NFCOMMON_API Image
{
    std::vector<Mipmap> mMipmaps;
    int mWidth;
    int mHeight;
    ImageFormat mFormat;

    static ImageTypeMap& mImageTypes() noexcept;
    friend class ImageType;

    bool DecompressDDS();
    bool CompressDDS(ImageFormat destFormat);
    bool GenerateMipmapsActual(MipmapFilter filterType, uint32 num);
    bool ConvertActual(ImageFormat destFormat);

public:
    Image();
    Image(const Image& src);
    Image(Image&& other) = delete;
    Image& operator=(const Image& other) = delete;
    Image& operator=(Image&& other) = delete;
    ~Image();

    /**
     * Free the image from memory.
     */
    void Release();

    /**
     * Generate mipmaps.
     * @param num Number of mipmaps to generate. Leave this argument default to generate a full set
     * @return true on success
     */
    bool GenerateMipmaps(MipmapFilter filterType, uint32 num = 0xFFFFFFFF);

    /**
     * Change pixel format. Currently converting from and to BC4-BC7 formats is unsupported.
     * @param destFormat Destination image format
     * @return true on success
     */
    bool Convert(ImageFormat destFormat);

    /**
     * Create an image with custom pixels buffer.
     * @param data Source data buffer
     * @param width Image width
     * @param height Image height
     * @param format Source data format
     * @return true on success
     */
    bool SetData(const void* data, uint32 width, uint32 height, ImageFormat format);

    /**
     * Load image from a data stream. Supported file formats: BMP, JPEG, PNG, DDS.
     * @param stream A pointer to a stream containing image file
     * @return true on success
     */
    bool Load(InputStream* stream);

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
    const void* GetData(uint32 mipmap = 0) const;

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
    const Mipmap* GetMipmap(uint32 id = 0) const;

    /**
     * Convert RGB pixel values to grayscale.
     * Accepted formats are only: RGB_UByte, RGBA_UByte, RGBA_Float
     * @return true on success
     */
    bool Grayscale();
};

} // namespace Common
} // namespace NFE

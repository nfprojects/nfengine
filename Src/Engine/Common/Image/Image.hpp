/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Image class declaration.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../Containers/DynArray.hpp"
#include "Mipmap.hpp"
#include "ImageFormat.hpp"


namespace NFE {
namespace Common {

class NFCOMMON_API Image
{
    DynArray<Mipmap> mMipmaps;
    int mWidth;
    int mHeight;
    ImageFormat mFormat;

public:
    Image();
    Image(const Image& src);
    Image(Image&& other) = delete;
    Image& operator=(const Image& other);
    Image& operator=(Image&& other);
    ~Image();

    static void ClearRegisteredTypesList();

    /**
     * Free the image from memory.
     */
    void Release();

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
     * Create an image from a set of mipmaps.
     */
    bool SetData(DynArray<Mipmap>&& mipmaps, uint32 width, uint32 height, ImageFormat format);

    void SetTexel(const Color& v, uint32 x, uint32 y);

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
    uint32 GetMipmapsNum() const;

    /**
     * Access n-th mipmap data.
     * @param id Mipmap level.
     */
    const Mipmap* GetMipmap(uint32 id = 0) const;
};

} // namespace Common
} // namespace NFE

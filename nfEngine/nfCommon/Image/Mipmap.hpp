/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Mipmap class declarations.
 */

#pragma once

#include "ImageFormat.hpp"
#include "../Math/Vector.hpp"
#include "../Memory/Buffer.hpp"


namespace NFE {
namespace Common {

using Color = Math::Vector;

class NFCOMMON_API Mipmap
{
protected:
    Buffer mData;

    uint32 mWidth;
    uint32 mHeight;

    typedef Color(Mipmap::*filterFunctor)(uint32, uint32, ImageFormat);
    Color FilterBox(uint32 x, uint32 y, ImageFormat fmt);
    Color FilterGammaCorrected(uint32 x, uint32 y, ImageFormat fmt);

    friend class Image;
public:
    Mipmap();
    Mipmap(const void* data, uint32 width, uint32 height, size_t dataSize);
    Mipmap(const Mipmap& other);
    Mipmap(Mipmap&& other);
    Mipmap& operator=(const Mipmap& other);
    Mipmap& operator=(Mipmap&& other) = delete;

    ~Mipmap();

    /**
     * Free the image from memory.
     */
    void Release();

    /**
     * Get mipmap content.
     */
    const void* GetData() const;

    /**
     * Get width of the mipmap (in pixels).
     */
    uint32 GetWidth() const;

    /**
     * Get height of the mipmap (in pixels).
     */
    uint32 GetHeight() const;

    /**
     * Get size of the mipmap data.
     */
    size_t GetDataSize() const;

    /**
     * Create a mipmap with custom pixels buffer.
     * @param data Source data buffer
     * @param width Image width
     * @param height Image height
     * @param dataSize Size of source data buffer
     * @return true on success
     */
    bool SetData(const void* data, uint32 width, uint32 height, size_t dataSize);

    /**
     * Get value of the texel on (x,y) position
     * @param x Texel position on the X axis
     * @param y Texel position on the Y axis
     * @param fmt Mipmap pixel format
     * @return Color object containing 4 values: red, green, blue, alpha
     */
    Color GetTexel(uint32 x, uint32 y, ImageFormat fmt) const;

    /**
     * Set value of the texel on (x,y) position
     * @param c Color object containing 4 values: red, green, blue, alpha
     * @param x Texel position on the X axis
     * @param y Texel position on the Y axis
     * @param fmt Mipmap pixel format
     */
    void SetTexel(const Color& c, uint32 x, uint32 y, ImageFormat fmt);
};

} // namespace Common
} // namespace NFE

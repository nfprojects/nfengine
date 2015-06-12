/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Mipmap class declarations.
 */

#pragma once

#include "ImageFormat.hpp"
#include "../Math/Math.hpp"

namespace NFE {
namespace Common {

 using namespace Math;
 typedef Vector Color;

 class NFCOMMON_API Mipmap
{
private:
    std::vector<uchar> mData;
    uint32 mWidth;
    uint32 mHeight;
    typedef std::function<Color(uint32, uint32, ImageFormat)> filterFunctor;

    Color FilterBox(uint32 x, uint32 y, ImageFormat fmt);
    Color FilterGammaCorrected(uint32 x, uint32 y, ImageFormat fmt);

    friend class Image;
public:
    Mipmap();
    Mipmap(void* data, uint32 width, uint32 height, size_t dataSize);
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
    const uint32 GetWidth() const;

    /**
     * Get height of the mipmap (in pixels).
     */
    const uint32 GetHeight() const;

    /**
     * Get size of the mipmap data (in 8bit chunks).
     */
    const size_t GetDataSize() const;

    /**
     * Create a mipmap with custom pixels buffer.
     * @param data Source data buffer
     * @param width Image width
     * @param height Image height
     * @param dataSize Size of source data buffer
     * @return true on success
     */
    bool SetData(void* data, uint32 width, uint32 height, size_t dataSize);

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

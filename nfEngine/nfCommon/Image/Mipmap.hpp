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

class Mipmap
{
private:
    uchar* mData;
    size_t mDataSize;
    uint32 mWidth;
    uint32 mHeight;

    Vector FilterBox(uint32 x, uint32 y);
    Vector FilterGammaCorrected(uint32 x, uint32 y);

public:
    Mipmap();
    Mipmap(void* data, uint32 width, uint32 height, size_t dataSize);
    Mipmap(const Mipmap& other);
    Mipmap(Mipmap&& other);
    //TODO assignment and move operators =delete;
    ~Mipmap();

    void Close();

    const void* GetData() const;
    const uint32 GetWidth() const;
    const uint32 GetHeight() const;
    const size_t GetDataSize() const;

    int SetData(void* data, uint32 width, uint32 height, size_t dataSize);
    Vector Filter(MipmapFilter filter, uint32 x, uint32 y);

    Vector GetTexel(uint32 x, uint32 y, ImageFormat fmt);
    bool SetTexel(const Color& c, uint32 x, uint32 y, ImageFormat fmt);
};

} // namespace Common
} // namespace NFE

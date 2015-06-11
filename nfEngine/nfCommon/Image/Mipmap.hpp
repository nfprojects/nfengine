/**
 * @file
 * @author mkkulagowski
 * @brief  Mipmap class declaration.
 */

#pragma once

#include "ImageFormat.hpp"
#include "../Math/Math.hpp"
//#include "../nfCommon.hpp"

//enum class ImageFormat;

namespace NFE {
namespace Common {

using namespace Math;

class Mipmap
{
private:
    uchar* mData;
    size_t mDataSize;
    uint32 mWidth;
    uint32 mHeight;

public:
    Mipmap();
    Mipmap(uchar* data, size_t dataSize, uint32 height, uint32 width);
    Mipmap(const Mipmap& other);
    Mipmap(Mipmap&& other);
    ~Mipmap();

    void Close();
    const uchar* GetData() const;
    const uint32 GetWidth() const;
    const uint32 GetHeight() const;
    const size_t GetDataSize() const;

    int SetData(uchar* data, size_t dataSize);
    void SetWidth(uint32 w);
    void SetHeight(uint32 h);
    void SetDataSize(size_t dataSize);

    int AllocData();
    int AllocData(size_t datasize);
    Vector GetTexel(uint32 x, uint32 y);
    bool SetTexel(const Vector& v, uint32 x, uint32 y);
    Vector GetTexel(uint32 x, uint32 y, ImageFormat fmt);
    bool SetTexel(const Vector& v, uint32 x, uint32 y, ImageFormat fmt);
};

typedef Mipmap* MipmapPtr;

} // namespace Common
} // namespace NFE

/**
 * @file
 * @author mkkulagowski
 * @brief  Mipmap class declaration.
 */

#pragma once

#include "Image.hpp"
#include "../nfCommon.hpp"

namespace NFE {
namespace Common {

using namespace Math;

class Mipmap
{
    uchar* mData;
    size_t mDataSize;
    uint32 mWidth;
    uint32 mHeight;
	
    Mipmap();
	Mipmap(uchar* data, size_t dataSize, uint32 height, uint32 width);
    Mipmap(const Mipmap& other);
    Mipmap(Mipmap&& other);
    ~Mipmap();
    
    int Close() noexcept;
	const uchar* GetData();
    const uint32 GetWidth();
    const uint32 GetHeight();
    const size_t GetDataSize();
    
    int SetData(uchar* data);
    void SetWidth(uint32 w);
    void SetHeight(uint32 h);
    void SetDataSize(size_t dataSize);
    
    int AllocData();
    int AllocData(size_t datasize);
    Vector GetTexel(uint32 x, uint32 y, ImageFormat fmt);
    bool SetTexel(uint32 x, uint32 y, ImageFormat fmt);
};
}
}
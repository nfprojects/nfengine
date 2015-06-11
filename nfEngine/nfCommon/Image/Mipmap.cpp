#include "Mipmap.hpp"
#include "Image.hpp"
#include "../PCH.hpp"

namespace NFE {
namespace Common {

Mipmap::Mipmap() :
    mData(nullptr),
    mDataSize(0),
    mWidth(0),
    mHeight(0),
{
}

Mipmap::Mipmap(uchar* data, size_t dataSize, uint32 height, uint32 width)
{
    SetData(data, dataSize);
    mHeight = height;
    mWidth = width;
}

Mipmap(const Mipmap& other)
    :Mipmap(other.mData, other.mDataSize, other.mHeight, other.mWidth)
{
}

Mipmap(Mipmap&& other)
    :Mipmap()
{
    std::swap(mData, other.mData);
    std::swap(mDataSize, other.mDataSize);
    std::swap(mHeight, other.mHeight);
    std::swap(mWidth, other.mWidth); 
}

Mipmap::~Mipmap()
{
    Close();
}

void Mipmap::Close() noexcept
{
    delete[] mData;
    this(new Mipmap);
    /*
    mData = nullptr;
    mDataSize = 0;
    mWidth = 0;
    mHeight = 0;
    */
}

const uchar* Mipmap::GetData()
{
    if (!mData)
        return mData;
    
    return nullptr;
}

const uint32 Mipmap::GetWidth()
{
    return mWidth;
}

const uint32 Mipmap::GetHeight()
{
    return mHeight;
}

const size_t Mipmap::GetDataSize()
{
    return mDataSize;
}

int Mipmap::AllocData()
{
    if (AllocData(mDataSize))
        return 1;
        
    return 0;
}

int Mipmap::AllocData(size_t datasize)
{
    delete[] mData;
    mData = new (std::nothrow) uchar[datasize];
    
    if (!mData)
        return 1;
     
    return 0;
}

int Mipmap::SetData(uchar* data, uint32 dataSize)
{
    mDataSize = dataSize;
    if (!mData)
        if (AllocData())
            return 1;
    memcpy(mData, data, dataSize);
    return 0;
}

void Mipmap::SetWidth(uint32 w)
{
    mWidth = w;
}

void Mipmap::SetHeight(uint32 h)
{
    mHeight = h;
}

void Mipmap::SetDataSize(size_t dataSize)
{
    mDataSize = dataSize;
}




Vector Mipmap::GetTexel(uint32 x, uint32 y, ImageFormat fmt)
{
    switch (fmt)
    {
        case ImageFormat::A_UByte:
        {
            const uchar* pSrc = (const uchar*)pData;
            uchar a = pSrc[y * width + x];
            return Vector(255.0f, 255.0f, 255.0f, (float)a) * g_Byte2Float;
        }

        case ImageFormat::R_UByte:
        {
            const uchar* pSrc = (const uchar*)pData;
            uchar r = pSrc[y * width + x];
            return Vector((float)r, 0.0f, 0.0f, 255.0f) * g_Byte2Float;
        }

        case ImageFormat::RGB_UByte:
        {
            const uchar* pSrc = (const uchar*)pData;
            uchar r = pSrc[3 * (y * width + x)];
            uchar g = pSrc[3 * (y * width + x) + 1];
            uchar b = pSrc[3 * (y * width + x) + 2];
            return Vector((float)r, (float)g, (float)b, 255.0f) * g_Byte2Float;
        }

        case ImageFormat::RGBA_UByte:
        {
            const uchar* pSrc = (const uchar*)pData;
            uchar r = pSrc[4 * (y * width + x)];
            uchar g = pSrc[4 * (y * width + x) + 1];
            uchar b = pSrc[4 * (y * width + x) + 2];
            uchar a = pSrc[4 * (y * width + x) + 3];
            return Vector((float)r, (float)g, (float)b, float(a)) * g_Byte2Float;
        }

        case ImageFormat::R_Float:
        {
            const float* pSrc = (const float*)pData;
            float r = pSrc[y * width + x];
            return Vector(r, 0.0f, 0.0f, 1.0f);
        }

        case ImageFormat::RGBA_Float:
        {
            const float* pSrc = (const float*)pData;
            return Vector(pSrc + 4 * (y * width + x));
        }
    }

    return Vector();
}

void Mipmap::SetTexel(const Vector& v, uint32 x, uint32 y, ImageFormat fmt)
{
    switch (fmt)
    {
        case ImageFormat::A_UByte:
        {
            uchar* pSrc = (uchar*)pData;
            pSrc[y * width + x] = (uchar)(v.f[3] * 255.0f);
            break;
        }

        case ImageFormat::R_UByte:
        {
            uchar* pSrc = (uchar*)pData;
            pSrc[y * width + x] = (uchar)(v.f[0] * 255.0f);
            break;
        }

        case ImageFormat::RGB_UByte:
        {
            uchar* pSrc = (uchar*)pData;
            pSrc[3 * (y * width + x)] = (uchar)(v.f[0] * 255.0f);
            pSrc[3 * (y * width + x) + 1] = (uchar)(v.f[1] * 255.0f);
            pSrc[3 * (y * width + x) + 2] = (uchar)(v.f[2] * 255.0f);
            break;
        }

        case ImageFormat::RGBA_UByte:
        {
            uchar* pSrc = (uchar*)pData;
            pSrc[4 * (y * width + x)] = (uchar)(v.f[0] * 255.0f);
            pSrc[4 * (y * width + x) + 1] = (uchar)(v.f[1] * 255.0f);
            pSrc[4 * (y * width + x) + 2] = (uchar)(v.f[2] * 255.0f);
            pSrc[4 * (y * width + x) + 3] = (uchar)(v.f[3] * 255.0f);
            break;
        }

        case ImageFormat::R_Float:
        {
            float* pSrc = (float*)pData;
            pSrc[y * width + x] = v.f[0];
            break;
        }

        case ImageFormat::RGBA_Float:
        {
            Float4* pSrc = (Float4*)pData;
            VectorStore(v, pSrc + y * width + x);
            break;
        }
    }
}
}
}
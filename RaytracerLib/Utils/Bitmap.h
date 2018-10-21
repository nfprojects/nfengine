#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"
#include "../Math/Vector8.h"
#include "../Utils/AlignmentAllocator.h"

namespace rt {

enum class TextureAddressMode : Uint8
{
    Repeat = 0,
    Clamp = 1,
    Border = 2,
};

enum class TextureFilterMode : Uint8
{
    NearestNeighbor = 0,
    Bilinear = 1,
};

struct SamplerDesc
{
    math::Vector4 borderColor;
    TextureAddressMode addressU = TextureAddressMode::Repeat;
    TextureAddressMode addressV = TextureAddressMode::Repeat;
    TextureFilterMode filter = TextureFilterMode::Bilinear;
    bool forceLinearSpace = false;
};

/**
 * Class representing 2D image/texture.
 */
class RT_ALIGN(64) RAYLIB_API Bitmap : public Aligned<64>
{
public:
    enum class Format : Uint8
    {
        Unknown = 0,
        R8_Uint,
        B8G8R8_Uint,
        B8G8R8A8_Uint,
        R32G32B32_Float,
        R32G32B32A32_Float,
        BC1,
        BC4,
        BC5,

        // TODO monochromatic, compressed, half-float, etc.
    };

    Bitmap();
    ~Bitmap();
    Bitmap(Bitmap&&) = default;
    Bitmap& operator = (Bitmap&&) = default;

    RT_FORCE_INLINE void* GetData() { return mData; }
    RT_FORCE_INLINE const void* GetData() const { return mData; }
    RT_FORCE_INLINE Uint32 GetWidth() const { return (Uint32)mWidth; }
    RT_FORCE_INLINE Uint32 GetHeight() const { return (Uint32)mHeight; }
    RT_FORCE_INLINE Format GetFormat() const { return mFormat; }

    static size_t GetDataSize(Uint32 width, Uint32 height, Format format);

    // initialize bitmap with data (or clean if passed nullptr)
    Bool Init(Uint32 width, Uint32 height, Format format, const void* data = nullptr, bool linearSpace = false);

    // load from file
    Bool Load(const char* path);

    // save to BMP file
    bool SaveBMP(const char* path, bool flipVertically) const;

    // release memory
    void Release();

    // Convert to tiled texture
    bool MakeTiled(Uint8 order);

    // calculate number of bits per pixel for given format
    static Uint32 BitsPerPixel(Format format);

    // get bitmap format description
    static const char* FormatToString(Format format);

    // set single pixel
    // TODO: this probably will be too slow
    void SetPixel(Uint32 x, Uint32 y, const math::Vector4& value);

    // get single pixel
    // TODO: this probably will be too slow
    math::Vector4 GetPixel(Uint32 x, Uint32 y, const bool forceLinearSpace = false) const;

    // sample the bitmap (including filtering and coordinates wrapping)
    math::Vector4 Sample(math::Vector4 coords, const SamplerDesc& sampler) const;

    // Note: format must be R32G32B32A32_Float
    void AccumulateFloat_Unsafe(const Uint32 x, const Uint32 y, const math::Vector4 value);

    // fill with zeros
    void Zero();

private:

    Bitmap(const Bitmap&) = delete;
    Bitmap& operator = (const Bitmap&) = delete;

    bool LoadBMP(FILE* file, const char* path);
    bool LoadDDS(FILE* file, const char* path);

    math::Vector4 mSize;
    Uint8* mData;
    Uint16 mWidth;
    Uint16 mHeight;
    Format mFormat;
    Uint8 mTileOrder;
    bool mLinearSpace;
};


} // namespace rt

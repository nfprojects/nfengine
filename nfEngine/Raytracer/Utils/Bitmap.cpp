#include "PCH.h"
#include "Bitmap.h"
#include "BlockCompression.h"
#include "../Color/ColorHelpers.h"
#include "../nfCommon/Memory/MemoryHelpers.hpp"
#include "../nfCommon/Math/Packed.hpp"
#include "../nfCommon/Math/Vector4Load.hpp"
#include "../nfCommon/Logger/Logger.hpp"
#include "../nfCommon/System/Timer.hpp"
#include "../nfCommon/System/Assertion.hpp"

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

static_assert(sizeof(Bitmap) <= 64, "Bitmap class is too big");

uint8 Bitmap::BitsPerPixel(Format format)
{
    switch (format)
    {
    case Format::Unknown:                   return 0;
    case Format::R8_UNorm:                  return 8 * sizeof(uint8);
    case Format::R8G8_UNorm:                return 8 * sizeof(uint8) * 2;
    case Format::B8G8R8_UNorm:              return 8 * sizeof(uint8) * 3;
    case Format::B8G8R8A8_UNorm:            return 8 * sizeof(uint8) * 4;
    case Format::R8G8B8A8_UNorm:            return 8 * sizeof(uint8) * 4;
    case Format::B8G8R8A8_UNorm_Palette:    return 8 * sizeof(uint8);
    case Format::B5G6R5_UNorm:              return 16;
    case Format::R16_UNorm:                 return 8 * sizeof(uint16);
    case Format::R16G16_UNorm:              return 8 * sizeof(uint16) * 2;
    case Format::R16G16B16A16_UNorm:        return 8 * sizeof(uint16) * 4;
    case Format::R32_Float:                 return 8 * sizeof(float);
    case Format::R32G32_Float:              return 8 * sizeof(float) * 2;
    case Format::R32G32B32_Float:           return 8 * sizeof(float) * 3;
    case Format::R32G32B32A32_Float:        return 8 * sizeof(float) * 4;
    case Format::R11G11B10_Float:           return 8 * 4;
    case Format::R16_Half:                  return 8 * sizeof(Half) * 1;
    case Format::R16G16_Half:               return 8 * sizeof(Half) * 2;
    case Format::R16G16B16_Half:            return 8 * sizeof(Half) * 3;
    case Format::R16G16B16A16_Half:         return 8 * sizeof(Half) * 4;
    case Format::R9G9B9E5_SharedExp:        return 9 + 9 + 9 + 5;
    case Format::BC1:                       return 4;
    case Format::BC4:                       return 4;
    case Format::BC5:                       return 8;
    }

    NFE_FATAL("Corrupted type");
    return 0;
}

const char* Bitmap::FormatToString(Format format)
{
    switch (format)
    {
    case Format::R8_UNorm:                  return "R8_UNorm";
    case Format::R8G8_UNorm:                return "R8G8_UNorm";
    case Format::B8G8R8_UNorm:              return "B8G8R8_UNorm";
    case Format::B8G8R8A8_UNorm:            return "B8G8R8A8_UNorm";
    case Format::R8G8B8A8_UNorm:            return "R8G8B8A8_UNorm";
    case Format::B8G8R8A8_UNorm_Palette:    return "B8G8R8A8_UNorm_Palette";
    case Format::B5G6R5_UNorm:              return "B5G6R5_UNorm";
    case Format::R16_UNorm:                 return "R16_UNorm";
    case Format::R16G16_UNorm:              return "R16G16_UNorm";
    case Format::R16G16B16A16_UNorm:        return "R16G16B16A16_UNorm";
    case Format::R32_Float:                 return "R32_Float";
    case Format::R32G32_Float:              return "R32G32_Float";
    case Format::R32G32B32_Float:           return "R32G32B32_Float";
    case Format::R32G32B32A32_Float:        return "R32G32B32A32_Float";
    case Format::R11G11B10_Float:           return "R11G11B10_Float";
    case Format::R16_Half:                  return "R16_Half";
    case Format::R16G16_Half:               return "R16G16_Half";
    case Format::R16G16B16_Half:            return "R16G16B16_Half";
    case Format::R16G16B16A16_Half:         return "R16G16B16A16_Half";
    case Format::R9G9B9E5_SharedExp:        return "R9G9B9E5_SharedExp";
    case Format::BC1:                       return "BC1";
    case Format::BC4:                       return "BC4";
    case Format::BC5:                       return "BC5";
    }

    NFE_FATAL("Corrupted type");
    return "<unknown>";
}

size_t Bitmap::ComputeDataSize(const InitData& initData)
{
    const uint32 stride = Max(initData.stride, ComputeDataStride(initData.width, initData.format));
    const uint64 dataSize = (uint64)initData.height * (uint64)stride;

    if (dataSize >= (uint64)std::numeric_limits<size_t>::max())
    {
        return std::numeric_limits<size_t>::max();
    }

    return (size_t)dataSize;
}

uint32 Bitmap::ComputeDataStride(uint32 width, Format format)
{
    return width * (uint64)BitsPerPixel(format) / 8;
}

Bitmap::Bitmap(const char* debugName)
    : mData(nullptr)
    , mPalette(nullptr)
    , mWidth(0)
    , mHeight(0)
    , mStride(0)
    , mPaletteSize(0)
    , mFormat(Format::Unknown)
    , mLinearSpace(false)
{
    NFE_ASSERT(debugName, "Invalid debug name");
    mDebugName = strdup(debugName);
}

Bitmap::~Bitmap()
{
    free(mDebugName);

    Release();
}

Bitmap::Bitmap(const Bitmap& other)
    : Bitmap(other.mDebugName)
{
    InitData data;
    data.data = other.mData;
    data.width = other.mWidth;
    data.height = other.mHeight;
    data.stride = other.mStride;
    data.format = other.mFormat;
    data.paletteSize = other.mPaletteSize;

    Init(data);
}

Bitmap& Bitmap::operator = (const Bitmap& other)
{
    Release();

    InitData data;
    data.data = other.mData;
    data.width = other.mWidth;
    data.height = other.mHeight;
    data.stride = other.mStride;
    data.format = other.mFormat;
    data.paletteSize = other.mPaletteSize;

    Init(data);

    return *this;
}

Bitmap::Bitmap(Bitmap&&) = default;

Bitmap& Bitmap::operator = (Bitmap&&) = default;

void Bitmap::Clear()
{
    if (mData)
    {
        memset(mData, 0, GetDataSize());
    }
}

void Bitmap::Release()
{
    if (mData)
    {
        NFE_FREE(mData);
        mData = nullptr;
    }

    if (mPalette)
    {
        NFE_FREE(mPalette);
        mData = nullptr;
    }

    mStride = 0;
    mWidth = 0;
    mHeight = 0;
    mPaletteSize = 0;
    mFormat = Format::Unknown;
}

bool Bitmap::Init(const InitData& initData)
{
    const size_t dataSize = ComputeDataSize(initData);
    if (dataSize == 0)
    {
        NFE_LOG_ERROR("Invalid bitmap format");
        return false;
    }
    if (dataSize == std::numeric_limits<size_t>::max())
    {
        NFE_LOG_ERROR("Bitmap is too big");
        return false;
    }

    Release();

    // align to cache line
    const uint32 marigin = NFE_CACHE_LINE_SIZE;

    mData = (uint8*)NFE_MALLOC(dataSize + marigin, NFE_CACHE_LINE_SIZE);

    if (!mData)
    {
        NFE_LOG_ERROR("Bitmap: Memory allocation failed");
        return false;
    }

    if (initData.data)
    {
        memcpy(mData, initData.data, dataSize);
    }

    if (initData.paletteSize > 0)
    {
        mPalette = (uint8*)NFE_MALLOC(sizeof(uint32) * (size_t)initData.paletteSize, NFE_CACHE_LINE_SIZE);
    }

    // clear marigin
    memset(mData + dataSize, 0, marigin);

    mStride = Max(initData.stride, ComputeDataStride(initData.width, initData.format));
    mWidth = initData.width;
    mHeight = initData.height;
    mFloatSize = Vector4::FromIntegers(initData.width, initData.height, initData.width, initData.height);
    mFormat = initData.format;
    mLinearSpace = initData.linearSpace;
    mPaletteSize = initData.paletteSize;

    return true;
}

bool Bitmap::Copy(Bitmap& target, const Bitmap& source)
{
    if (target.mWidth != source.mWidth || target.mHeight != source.mHeight)
    {
        NFE_LOG_ERROR("Bitmap copy failed: bitmaps have different dimensions");
        return false;
    }

    if (target.mFormat != source.mFormat)
    {
        NFE_LOG_ERROR("Bitmap copy failed: bitmaps have different formats");
        return false;
    }

    if (target.mPaletteSize != source.mPaletteSize)
    {
        NFE_LOG_ERROR("Bitmap copy failed: bitmaps have palettes");
        return false;
    }

    if (target.mStride == source.mStride)
    {
        NFE_ASSERT(target.GetDataSize() == source.GetDataSize());
        memcpy(target.GetData(), source.GetData(), source.GetDataSize());
    }
    else
    {
        uint32 rowSize = ComputeDataStride(source.mWidth, source.mFormat);
        for (size_t i = 0; i < source.mHeight; ++i)
        {
            memcpy(target.GetData() + size_t(target.mStride) * i, source.GetData() + size_t(source.mStride) * i, rowSize);
        }
    }

    if (source.mPalette)
    {
        memcpy(target.mPalette, source.mPalette, sizeof(uint32) * source.mPaletteSize);
    }

    return true;
}

bool Bitmap::Load(const char* path)
{
    Timer timer;

    FILE* file = fopen(path, "rb");
    if (!file)
    {
        NFE_LOG_ERROR("Failed to load source image from file '%hs'", path);
        return false;
    }

    if (!LoadBMP(file, path))
    {
        fseek(file, SEEK_SET, 0);

        if (!LoadDDS(file, path))
        {
            fseek(file, SEEK_SET, 0);

            if (!LoadEXR(file, path))
            {
                NFE_LOG_ERROR("Failed to load '%hs' - unknown format", path);
                fclose(file);
                return false;
            }
        }
    }

    fclose(file);

    const float elapsedTime = static_cast<float>(1000.0 * timer.Stop());
    NFE_LOG_INFO("Bitmap '%hs' loaded in %.3fms: width=%u, height=%u, format=%s, %s",
        path, elapsedTime, mWidth, mHeight, FormatToString(mFormat),
        mLinearSpace ? "linear-space" : "gamma-space");
    return true;
}

const Vector4 Bitmap::GetPixel(uint32 x, uint32 y, const bool forceLinearSpace) const
{
    NFE_ASSERT(x < mWidth);
    NFE_ASSERT(y < mHeight);

    const size_t rowOffset = static_cast<size_t>(mStride) * static_cast<size_t>(y);
    const uint8* rowData = mData + rowOffset;

    Vector4 color;
    switch (mFormat)
    {
    case Format::R8_UNorm:
    {
        const uint32 value = rowData[x];
        color = Vector4::FromInteger(value) * (1.0f / 255.0f);
        break;
    }

    case Format::R8G8_UNorm:
    {
        color = Vector4_Load_2xUint8_Norm(rowData + 2u * (size_t)x);
        break;
    }

    case Format::B8G8R8_UNorm:
    {
        const uint8* source = rowData + 3u * (size_t)x;
        color = Vector4_LoadBGR_UNorm(source);
        break;
    }

    case Format::B8G8R8A8_UNorm:
    {
        const uint8* source = rowData + 4u * (size_t)x;
        color = Vector4_Load_4xUint8(source).Swizzle<2, 1, 0, 3>() * (1.0f / 255.0f);
        break;
    }

    case Format::R8G8B8A8_UNorm:
    {
        const uint8* source = rowData + 4u * (size_t)x;
        color = Vector4_Load_4xUint8(source) * (1.0f / 255.0f);
        break;
    }

    case Format::B8G8R8A8_UNorm_Palette:
    {
        const size_t paletteIndex = rowData[x];
        const uint8* source = mPalette + 4u * paletteIndex;
        color = Vector4_Load_4xUint8(source).Swizzle<2, 1, 0, 3>() * (1.0f / 255.0f);
        break;
    }

    case Format::B5G6R5_UNorm:
    {
        const uint16* source = reinterpret_cast<const uint16*>(rowData) + (size_t)x;
        color = Vector4_Load_B5G6R5_Norm(source);
        break;
    }

    case Format::R16_UNorm:
    {
        const uint16* source = reinterpret_cast<const uint16*>(rowData) + (size_t)x;
        color = Vector4::FromInteger(*source) * (1.0f / 65535.0f);
        break;
    }

    case Format::R16G16_UNorm:
    {
        const uint16* source = reinterpret_cast<const uint16*>(rowData) + 2u * (size_t)x;
        color = Vector4_Load_2xUint16_Norm(source);
        break;
    }

    case Format::R16G16B16A16_UNorm:
    {
        const uint16* source = reinterpret_cast<const uint16*>(rowData) + 4u * (size_t)x;
        color = Vector4_Load_4xUint16(source) * (1.0f / 65535.0f);
        break;
    }

    case Format::R32_Float:
    {
        const float* source = reinterpret_cast<const float*>(rowData) + (size_t)x;
        color = Vector4(*source);
        break;
    }

    case Format::R32G32_Float:
    {
        const float* source = reinterpret_cast<const float*>(rowData) + 2u * (size_t)x;
        color = Vector4(source) & Vector4::MakeMask<1, 1, 0, 0>();
        break;
    }

    case Format::R32G32B32_Float:
    {
        const float* source = reinterpret_cast<const float*>(rowData) + 3u * (size_t)x;
        color = Vector4(source) & Vector4::MakeMask<1, 1, 1, 0>();
        break;
    }

    case Format::R32G32B32A32_Float:
    {
        const Vector4* source = reinterpret_cast<const Vector4*>(rowData) + (size_t)x;
        color = *source;
        break;
    }

    case Format::R16_Half:
    {
        const Half* source = reinterpret_cast<const Half*>(rowData) + (size_t)x;
        color = Vector4(source->ToFloat());
        break;
    }

    case Format::R16G16_Half:
    {
        const Half* source = reinterpret_cast<const Half*>(rowData) + 2u * (size_t)x;
        color = Vector4_Load_Half2(source);
        break;
    }

    case Format::R16G16B16_Half:
    {
        const Half* source = reinterpret_cast<const Half*>(rowData) + 3u * (size_t)x;
        color = Vector4_Load_Half4(source) & Vector4::MakeMask<1, 1, 1, 0>();
        break;
    }

    case Format::R16G16B16A16_Half:
    {
        const Half* source = reinterpret_cast<const Half*>(rowData) + 4u * (size_t)x;
        color = Vector4_Load_Half4(source);
        break;
    }

    case Format::R9G9B9E5_SharedExp:
    {
        const SharedExpFloat3* source = reinterpret_cast<const SharedExpFloat3*>(rowData) + (size_t)x;
        color = source->ToVector();
        break;
    }

    case Format::R11G11B10_Float:
    {
        const PackedFloat3* source = reinterpret_cast<const PackedFloat3*>(rowData) + (size_t)x;
        color = source->ToVector();
        break;
    }

    case Format::BC1:
    {
        color = DecodeBC1(reinterpret_cast<const uint8*>(mData), x, y, mWidth);
        break;
    }

    case Format::BC4:
    {
        color = DecodeBC4(reinterpret_cast<const uint8*>(mData), x, y, mWidth);
        break;
    }

    case Format::BC5:
    {
        color = DecodeBC5(reinterpret_cast<const uint8*>(mData), x, y, mWidth);
        break;
    }

    default:
    {
        NFE_FATAL("Unsupported bitmap format");
        color = Vector4::Zero();
    }
    }

    (void)forceLinearSpace;
    if (!mLinearSpace /*&& !forceLinearSpace*/)
    {
        color = Convert_sRGB_To_Linear(color);
    }

    return color;
}

void Bitmap::GetPixelBlock(const VectorInt4 coords, Vector4* outColors, const bool forceLinearSpace) const
{
    NFE_ASSERT(coords.x >= 0 && coords.x < (int32)mWidth);
    NFE_ASSERT(coords.y >= 0 && coords.y < (int32)mHeight);
    NFE_ASSERT(coords.z >= 0 && coords.z < (int32)mWidth);
    NFE_ASSERT(coords.w >= 0 && coords.w < (int32)mHeight);

    const uint8* rowData0 = mData + mStride * static_cast<size_t>(coords.y);
    const uint8* rowData1 = mData + mStride * static_cast<size_t>(coords.w);

    Vector4 color[4];

    switch (mFormat)
    {
    case Format::R8_UNorm:
    {
        constexpr float scale = 1.0f / 255.0f;
        const uint32 value0 = rowData0[(uint32)coords.x];
        const uint32 value1 = rowData0[(uint32)coords.z];
        const uint32 value2 = rowData1[(uint32)coords.x];
        const uint32 value3 = rowData1[(uint32)coords.z];
        const Vector4 values = Vector4::FromIntegers(value0, value1, value2, value3) * scale;
        color[0] = values.SplatX();
        color[1] = values.SplatY();
        color[2] = values.SplatZ();
        color[3] = values.SplatW();
        break;
    }

    case Format::R8G8_UNorm:
    {
        const VectorInt4 offsets = coords << 1; // offset = 2 * coords
        color[0] = Vector4_Load_2xUint8_Norm(rowData0 + (uint32)offsets.x);
        color[1] = Vector4_Load_2xUint8_Norm(rowData0 + (uint32)offsets.z);
        color[2] = Vector4_Load_2xUint8_Norm(rowData1 + (uint32)offsets.x);
        color[3] = Vector4_Load_2xUint8_Norm(rowData1 + (uint32)offsets.z);
        break;
    }

    case Format::B8G8R8_UNorm:
    {
        const VectorInt4 offsets = coords + (coords << 1); // offset = 3 * coords
        color[0] = Vector4_LoadBGR_UNorm(rowData0 + (uint32)offsets.x);
        color[1] = Vector4_LoadBGR_UNorm(rowData0 + (uint32)offsets.z);
        color[2] = Vector4_LoadBGR_UNorm(rowData1 + (uint32)offsets.x);
        color[3] = Vector4_LoadBGR_UNorm(rowData1 + (uint32)offsets.z);
        break;
    }

    case Format::B8G8R8A8_UNorm:
    {
        constexpr float scale = 1.0f / 255.0f;
        const VectorInt4 offsets = coords << 2; // offset = 4 * coords
        color[0] = Vector4_Load_4xUint8(rowData0 + (uint32)offsets.x).Swizzle<2, 1, 0, 3>() * scale;
        color[1] = Vector4_Load_4xUint8(rowData0 + (uint32)offsets.z).Swizzle<2, 1, 0, 3>() * scale;
        color[2] = Vector4_Load_4xUint8(rowData1 + (uint32)offsets.x).Swizzle<2, 1, 0, 3>() * scale;
        color[3] = Vector4_Load_4xUint8(rowData1 + (uint32)offsets.z).Swizzle<2, 1, 0, 3>() * scale;
        break;
    }

    case Format::R8G8B8A8_UNorm:
    {
        constexpr float scale = 1.0f / 255.0f;
        const VectorInt4 offsets = coords << 2; // offset = 4 * coords
        color[0] = Vector4_Load_4xUint8(rowData0 + (uint32)offsets.x) * scale;
        color[1] = Vector4_Load_4xUint8(rowData0 + (uint32)offsets.z) * scale;
        color[2] = Vector4_Load_4xUint8(rowData1 + (uint32)offsets.x) * scale;
        color[3] = Vector4_Load_4xUint8(rowData1 + (uint32)offsets.z) * scale;
        break;
    }

    case Format::B8G8R8A8_UNorm_Palette:
    {
        constexpr float scale = 1.0f / 255.0f;
        const uint8* source0 = mPalette + 4u * rowData0[coords.x];
        const uint8* source1 = mPalette + 4u * rowData0[coords.z];
        const uint8* source2 = mPalette + 4u * rowData1[coords.x];
        const uint8* source3 = mPalette + 4u * rowData1[coords.z];
        color[0] = Vector4_Load_4xUint8(source0).Swizzle<2, 1, 0, 3>() * scale;
        color[1] = Vector4_Load_4xUint8(source1).Swizzle<2, 1, 0, 3>() * scale;
        color[2] = Vector4_Load_4xUint8(source2).Swizzle<2, 1, 0, 3>() * scale;
        color[3] = Vector4_Load_4xUint8(source3).Swizzle<2, 1, 0, 3>() * scale;
        break;
    }

    case Format::B5G6R5_UNorm:
    {
        const VectorInt4 offsets = coords << 1; // offset = 2 * coords
        const uint16* source0 = reinterpret_cast<const uint16*>(rowData0 + (uint32)offsets.x);
        const uint16* source1 = reinterpret_cast<const uint16*>(rowData0 + (uint32)offsets.z);
        const uint16* source2 = reinterpret_cast<const uint16*>(rowData1 + (uint32)offsets.x);
        const uint16* source3 = reinterpret_cast<const uint16*>(rowData1 + (uint32)offsets.z);
        color[0] = Vector4_Load_B5G6R5_Norm(source0);
        color[1] = Vector4_Load_B5G6R5_Norm(source1);
        color[2] = Vector4_Load_B5G6R5_Norm(source2);
        color[3] = Vector4_Load_B5G6R5_Norm(source3);
        break;
    }

    case Format::R16_UNorm:
    {
        constexpr float scale = 1.0f / 65535.0f;
        const uint32 value0 = reinterpret_cast<const uint16*>(rowData0)[(uint32)coords.x];
        const uint32 value1 = reinterpret_cast<const uint16*>(rowData0)[(uint32)coords.z];
        const uint32 value2 = reinterpret_cast<const uint16*>(rowData1)[(uint32)coords.x];
        const uint32 value3 = reinterpret_cast<const uint16*>(rowData1)[(uint32)coords.z];
        const Vector4 values = Vector4::FromIntegers(value0, value1, value2, value3) * scale;
        color[0] = values.SplatX();
        color[1] = values.SplatY();
        color[2] = values.SplatZ();
        color[3] = values.SplatW();
        break;
    }

    case Format::R16G16_UNorm:
    {
        const VectorInt4 offsets = coords << 2; // offset = 4 * coords
        const uint16* source0 = reinterpret_cast<const uint16*>(rowData0 + (uint32)offsets.x);
        const uint16* source1 = reinterpret_cast<const uint16*>(rowData0 + (uint32)offsets.z);
        const uint16* source2 = reinterpret_cast<const uint16*>(rowData1 + (uint32)offsets.x);
        const uint16* source3 = reinterpret_cast<const uint16*>(rowData1 + (uint32)offsets.z);
        color[0] = Vector4_Load_2xUint16_Norm(source0);
        color[1] = Vector4_Load_2xUint16_Norm(source1);
        color[2] = Vector4_Load_2xUint16_Norm(source2);
        color[3] = Vector4_Load_2xUint16_Norm(source3);
        break;
    }

    case Format::R16G16B16A16_UNorm:
    {
        constexpr float scale = 1.0f / 65535.0f;
        const VectorInt4 offsets = coords << 3; // offset = 8 * coords
        const uint16* source0 = reinterpret_cast<const uint16*>(rowData0 + (uint32)offsets.x);
        const uint16* source1 = reinterpret_cast<const uint16*>(rowData0 + (uint32)offsets.z);
        const uint16* source2 = reinterpret_cast<const uint16*>(rowData1 + (uint32)offsets.x);
        const uint16* source3 = reinterpret_cast<const uint16*>(rowData1 + (uint32)offsets.z);
        color[0] = Vector4_Load_4xUint16(source0) * scale;
        color[1] = Vector4_Load_4xUint16(source1) * scale;
        color[2] = Vector4_Load_4xUint16(source2) * scale;
        color[3] = Vector4_Load_4xUint16(source3) * scale;
        break;
    }

    case Format::R32_Float:
    {
        const float* source0 = reinterpret_cast<const float*>(rowData0) + (uint32)coords.x;
        const float* source1 = reinterpret_cast<const float*>(rowData0) + (uint32)coords.z;
        const float* source2 = reinterpret_cast<const float*>(rowData1) + (uint32)coords.x;
        const float* source3 = reinterpret_cast<const float*>(rowData1) + (uint32)coords.z;
        color[0] = Vector4(*source0);
        color[1] = Vector4(*source1);
        color[2] = Vector4(*source2);
        color[3] = Vector4(*source3);
        break;
    }

    case Format::R32G32_Float:
    {
        const VectorInt4 offsets = coords << 3; // offset = 8 * coords
        const float* source0 = reinterpret_cast<const float*>(rowData0 + (uint32)offsets.x);
        const float* source1 = reinterpret_cast<const float*>(rowData0 + (uint32)offsets.z);
        const float* source2 = reinterpret_cast<const float*>(rowData1 + (uint32)offsets.x);
        const float* source3 = reinterpret_cast<const float*>(rowData1 + (uint32)offsets.z);
        color[0] = Vector4(source0) & Vector4::MakeMask<1, 1, 0, 0>();
        color[1] = Vector4(source1) & Vector4::MakeMask<1, 1, 0, 0>();
        color[2] = Vector4(source2) & Vector4::MakeMask<1, 1, 0, 0>();
        color[3] = Vector4(source3) & Vector4::MakeMask<1, 1, 0, 0>();
        break;
    }

    case Format::R32G32B32_Float:
    {
        const VectorInt4 offsets = coords + (coords << 1); // offset = 3 * coords
        const float* source0 = reinterpret_cast<const float*>(rowData0) + (uint32)offsets.x;
        const float* source1 = reinterpret_cast<const float*>(rowData0) + (uint32)offsets.z;
        const float* source2 = reinterpret_cast<const float*>(rowData1) + (uint32)offsets.x;
        const float* source3 = reinterpret_cast<const float*>(rowData1) + (uint32)offsets.z;
        color[0] = Vector4(source0) & Vector4::MakeMask<1, 1, 1, 0>();
        color[1] = Vector4(source1) & Vector4::MakeMask<1, 1, 1, 0>();
        color[2] = Vector4(source2) & Vector4::MakeMask<1, 1, 1, 0>();
        color[3] = Vector4(source3) & Vector4::MakeMask<1, 1, 1, 0>();
        break;
    }

    case Format::R32G32B32A32_Float:
    {
        color[0] = reinterpret_cast<const Vector4*>(rowData0)[coords.x];
        color[1] = reinterpret_cast<const Vector4*>(rowData0)[coords.z];
        color[2] = reinterpret_cast<const Vector4*>(rowData1)[coords.x];
        color[3] = reinterpret_cast<const Vector4*>(rowData1)[coords.z];
        break;
    }

    case Format::R16_Half:
    {
        const VectorInt4 offsets = coords << 1; // offset = 2 * coords
        const Half* source0 = reinterpret_cast<const Half*>(rowData0 + (uint32)offsets.x);
        const Half* source1 = reinterpret_cast<const Half*>(rowData0 + (uint32)offsets.z);
        const Half* source2 = reinterpret_cast<const Half*>(rowData1 + (uint32)offsets.x);
        const Half* source3 = reinterpret_cast<const Half*>(rowData1 + (uint32)offsets.z);
        color[0] = Vector4(source0->ToFloat());
        color[1] = Vector4(source1->ToFloat());
        color[2] = Vector4(source2->ToFloat());
        color[3] = Vector4(source3->ToFloat());
        break;
    }

    case Format::R16G16_Half:
    {
        const VectorInt4 offsets = coords << 2; // offset = 4 * coords
        const Half* source0 = reinterpret_cast<const Half*>(rowData0 + (uint32)offsets.x);
        const Half* source1 = reinterpret_cast<const Half*>(rowData0 + (uint32)offsets.z);
        const Half* source2 = reinterpret_cast<const Half*>(rowData1 + (uint32)offsets.x);
        const Half* source3 = reinterpret_cast<const Half*>(rowData1 + (uint32)offsets.z);
        color[0] = Vector4_Load_Half2(source0);
        color[1] = Vector4_Load_Half2(source1);
        color[2] = Vector4_Load_Half2(source2);
        color[3] = Vector4_Load_Half2(source3);
        break;
    }

    case Format::R16G16B16_Half:
    {
        const VectorInt4 offsets = (coords << 2) + (coords << 1); // offset = 6 * coords
        const Half* source0 = reinterpret_cast<const Half*>(rowData0 + (uint32)offsets.x);
        const Half* source1 = reinterpret_cast<const Half*>(rowData0 + (uint32)offsets.z);
        const Half* source2 = reinterpret_cast<const Half*>(rowData1 + (uint32)offsets.x);
        const Half* source3 = reinterpret_cast<const Half*>(rowData1 + (uint32)offsets.z);
        color[0] = Vector4_Load_Half4(source0) & Vector4::MakeMask<1, 1, 1, 0>();
        color[1] = Vector4_Load_Half4(source1) & Vector4::MakeMask<1, 1, 1, 0>();
        color[2] = Vector4_Load_Half4(source2) & Vector4::MakeMask<1, 1, 1, 0>();
        color[3] = Vector4_Load_Half4(source3) & Vector4::MakeMask<1, 1, 1, 0>();
        break;
    }

    case Format::R16G16B16A16_Half:
    {
        const VectorInt4 offsets = coords << 3; // offset = 8 * coords
        const Half* source0 = reinterpret_cast<const Half*>(rowData0 + (uint32)offsets.x);
        const Half* source1 = reinterpret_cast<const Half*>(rowData0 + (uint32)offsets.z);
        const Half* source2 = reinterpret_cast<const Half*>(rowData1 + (uint32)offsets.x);
        const Half* source3 = reinterpret_cast<const Half*>(rowData1 + (uint32)offsets.z);
        color[0] = Vector4_Load_Half4(source0);
        color[1] = Vector4_Load_Half4(source1);
        color[2] = Vector4_Load_Half4(source2);
        color[3] = Vector4_Load_Half4(source3);
        break;
    }
    
    case Format::R9G9B9E5_SharedExp:
    {
        const VectorInt4 offsets = coords << 2; // offset = 4 * coords
        const SharedExpFloat3* source0 = reinterpret_cast<const SharedExpFloat3*>(rowData0 + (uint32)offsets.x);
        const SharedExpFloat3* source1 = reinterpret_cast<const SharedExpFloat3*>(rowData0 + (uint32)offsets.z);
        const SharedExpFloat3* source2 = reinterpret_cast<const SharedExpFloat3*>(rowData1 + (uint32)offsets.x);
        const SharedExpFloat3* source3 = reinterpret_cast<const SharedExpFloat3*>(rowData1 + (uint32)offsets.z);
        // TODO vectorize
        color[0] = source0->ToVector();
        color[1] = source1->ToVector();
        color[2] = source2->ToVector();
        color[3] = source3->ToVector();
        break;
    }

    case Format::R11G11B10_Float:
    {
        const VectorInt4 offsets = coords << 2; // offset = 4 * coords
        const PackedFloat3* source0 = reinterpret_cast<const PackedFloat3*>(rowData0 + (uint32)offsets.x);
        const PackedFloat3* source1 = reinterpret_cast<const PackedFloat3*>(rowData0 + (uint32)offsets.z);
        const PackedFloat3* source2 = reinterpret_cast<const PackedFloat3*>(rowData1 + (uint32)offsets.x);
        const PackedFloat3* source3 = reinterpret_cast<const PackedFloat3*>(rowData1 + (uint32)offsets.z);
        // TODO vectorize
        color[0] = source0->ToVector();
        color[1] = source1->ToVector();
        color[2] = source2->ToVector();
        color[3] = source3->ToVector();
        break;
    }

    case Format::BC1:
    {
        color[0] = DecodeBC1(mData, coords.x, coords.y, mWidth);
        color[1] = DecodeBC1(mData, coords.z, coords.y, mWidth);
        color[2] = DecodeBC1(mData, coords.x, coords.w, mWidth);
        color[3] = DecodeBC1(mData, coords.z, coords.w, mWidth);
        break;
    }

    case Format::BC4:
    {
        color[0] = DecodeBC4(mData, coords.x, coords.y, mWidth);
        color[1] = DecodeBC4(mData, coords.z, coords.y, mWidth);
        color[2] = DecodeBC4(mData, coords.x, coords.w, mWidth);
        color[3] = DecodeBC4(mData, coords.z, coords.w, mWidth);
        break;
    }

    case Format::BC5:
    {
        color[0] = DecodeBC5(mData, coords.x, coords.y, mWidth);
        color[1] = DecodeBC5(mData, coords.z, coords.y, mWidth);
        color[2] = DecodeBC5(mData, coords.x, coords.w, mWidth);
        color[3] = DecodeBC5(mData, coords.z, coords.w, mWidth);
        break;
    }

    default:
    {
        NFE_FATAL("Unsupported bitmap format");
    }
    }

    (void)forceLinearSpace;
    if (!mLinearSpace /*&& !forceLinearSpace*/)
    {
        color[0] = Convert_sRGB_To_Linear(color[0]);
        color[1] = Convert_sRGB_To_Linear(color[1]);
        color[2] = Convert_sRGB_To_Linear(color[2]);
        color[3] = Convert_sRGB_To_Linear(color[3]);
    }

    outColors[0] = color[0];
    outColors[1] = color[1];
    outColors[2] = color[2];
    outColors[3] = color[3];
}

bool Bitmap::Scale(const Math::Vector4& factor)
{
    if (mFormat == Format::R32G32B32_Float)
    {
        for (uint32 y = 0; y < mHeight; ++y)
        {
            for (uint32 x = 0; x < mWidth; ++x)
            {
                Float3& pixelPtr = GetPixelRef<Float3>(x, y);
                pixelPtr.x *= factor.x;
                pixelPtr.y *= factor.y;
                pixelPtr.z *= factor.z;
            }
        }
        return true;
    }
    else if (mFormat == Format::R32G32B32A32_Float)
    {
        for (uint32 y = 0; y < mHeight; ++y)
        {
            for (uint32 x = 0; x < mWidth; ++x)
            {
                Vector4& pixelPtr = GetPixelRef<Vector4>(x, y);
                pixelPtr *= factor;
            }
        }
        return true;
    }

    NFE_LOG_ERROR("Bitmap::Scale: Unsupported texture format");
    return false;
}


} // namespace RT
} // namespace NFE

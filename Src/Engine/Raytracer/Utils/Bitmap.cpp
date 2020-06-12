#include "PCH.h"
#include "Bitmap.h"
#include "BlockCompression.h"
#include "../Common/Math/ColorHelpers.hpp"
#include "../Common/Math/PackedLoadVec4f.hpp"
#include "../Common/Logger/Logger.hpp"
#include "../Common/System/Timer.hpp"

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

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
    case Format::R10G10B10A2_UNorm:         return 32;
    case Format::B5G6R5_UNorm:              return 16;
    case Format::B4G4R4A4_UNorm:            return 16;
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
    case Format::R10G10B10A2_UNorm:         return "R10G10B10A2_UNorm";
    case Format::B5G6R5_UNorm:              return "B5G6R5_UNorm";
    case Format::B4G4R4A4_UNorm:            return "B4G4R4A4_UNorm";
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
    const uint64 dataSize = (uint64)initData.depth * (uint64)initData.height * (uint64)stride;

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
    data.width = other.GetWidth();
    data.height = other.GetHeight();
    data.depth = other.GetDepth();
    data.stride = other.GetStride();
    data.format = other.mFormat;
    data.paletteSize = other.mPaletteSize;

    Init(data);
}

Bitmap& Bitmap::operator = (const Bitmap& other)
{
    Release();

    InitData data;
    data.data = other.mData;
    data.width = other.GetWidth();
    data.height = other.GetHeight();
    data.depth = other.GetDepth();
    data.stride = other.GetStride();
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

    mSize = Vec4ui::Zero();
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

    mSize.x = initData.width;
    mSize.y = initData.height;
    mSize.z = initData.depth;
    mSize.w = Max(initData.stride, ComputeDataStride(initData.width, initData.format)); // stride
    mFloatSize = Vec4f::FromIntegers(initData.width, initData.height, initData.depth, 0);
    mFormat = initData.format;
    mLinearSpace = initData.linearSpace;
    mPaletteSize = initData.paletteSize;

    return true;
}

bool Bitmap::Copy(Bitmap& target, const Bitmap& source)
{
    if ((target.GetSize() != target.GetSize()).Any3())
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

    if (target.GetStride() == source.GetStride())
    {
        NFE_ASSERT(target.GetDataSize() == source.GetDataSize(), "");
        memcpy(target.GetData(), source.GetData(), source.GetDataSize());
    }
    else
    {
        uint32 rowSize = ComputeDataStride(source.GetWidth(), source.mFormat);
        size_t numRows = (size_t)source.GetHeight() * (size_t)source.GetDepth();
        for (size_t i = 0; i < numRows; ++i)
        {
            memcpy(target.GetData() + size_t(target.GetStride()) * i, source.GetData() + size_t(source.GetStride()) * i, rowSize);
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
                //if (!LoadVDB(file, path))
                //{
                //    NFE_LOG_ERROR("Failed to load '%hs' - unknown format", path);
                //    fclose(file);
                //    return false;
                //}
                return false;
            }
        }
    }

    fclose(file);

    const float elapsedTime = static_cast<float>(1000.0 * timer.Stop());
    NFE_LOG_INFO("Bitmap '%hs' loaded in %.3fms: width=%u, height=%u, depth=%u, format=%s, %s",
        path, elapsedTime, GetWidth(), GetHeight(), GetDepth(), FormatToString(mFormat),
        mLinearSpace ? "linear-space" : "gamma-space");
    return true;
}

const Vec4f Bitmap::GetPixel(uint32 x, uint32 y) const
{
    NFE_ASSERT((x < GetWidth()) && (y < GetHeight()), "");

    const size_t rowOffset = static_cast<size_t>(GetStride()) * static_cast<size_t>(y);
    const uint8* rowData = mData + rowOffset;

    Vec4f color;
    switch (mFormat)
    {
    case Format::R8_UNorm:
    {
        const uint32 value = rowData[x];
        color = Vec4f::FromInteger(value) * (1.0f / 255.0f);
        break;
    }

    case Format::R8G8_UNorm:
    {
        color = Vec4f_Load_2xUint8_Norm(rowData + 2u * (size_t)x);
        break;
    }

    case Format::B8G8R8_UNorm:
    {
        const uint8* source = rowData + 3u * (size_t)x;
        color = Vec4f_LoadBGR_UNorm(source);
        break;
    }

    case Format::B8G8R8A8_UNorm:
    {
        const uint8* source = rowData + 4u * (size_t)x;
        color = Vec4f_Load_4xUint8(source).Swizzle<2, 1, 0, 3>() * (1.0f / 255.0f);
        break;
    }

    case Format::R8G8B8A8_UNorm:
    {
        const uint8* source = rowData + 4u * (size_t)x;
        color = Vec4f_Load_4xUint8(source) * (1.0f / 255.0f);
        break;
    }

    case Format::B8G8R8A8_UNorm_Palette:
    {
        const size_t paletteIndex = rowData[x];
        const uint8* source = mPalette + 4u * paletteIndex;
        color = Vec4f_Load_4xUint8(source).Swizzle<2, 1, 0, 3>() * (1.0f / 255.0f);
        break;
    }

    case Format::B5G6R5_UNorm:
    {
        const Packed_5_6_5* source = reinterpret_cast<const Packed_5_6_5*>(rowData);
        color = LoadVec4fUNorm(source[x]);
        break;
    }

    case Format::B4G4R4A4_UNorm:
    {
        const Packed_4_4_4_4* source = reinterpret_cast<const Packed_4_4_4_4*>(rowData);
        color = LoadVec4fUNorm(source[x]);
        break;
    }

    case Format::R10G10B10A2_UNorm:
    {
        const Packed_10_10_10_2* source = reinterpret_cast<const Packed_10_10_10_2*>(rowData);
        color = LoadVec4fUNorm(source[x]);
        break;
    }

    case Format::R16_UNorm:
    {
        const uint16* source = reinterpret_cast<const uint16*>(rowData) + (size_t)x;
        color = Vec4f::FromInteger(*source) * (1.0f / 65535.0f);
        break;
    }

    case Format::R16G16_UNorm:
    {
        const uint16* source = reinterpret_cast<const uint16*>(rowData) + 2u * (size_t)x;
        color = Vec4f_Load_2xUint16_Norm(source);
        break;
    }

    case Format::R16G16B16A16_UNorm:
    {
        const uint16* source = reinterpret_cast<const uint16*>(rowData) + 4u * (size_t)x;
        color = Vec4f_Load_4xUint16(source) * (1.0f / 65535.0f);
        break;
    }

    case Format::R32_Float:
    {
        const float* source = reinterpret_cast<const float*>(rowData) + (size_t)x;
        color = Vec4f(*source);
        break;
    }

    case Format::R32G32_Float:
    {
        const float* source = reinterpret_cast<const float*>(rowData) + 2u * (size_t)x;
        color = Vec4f(source) & Vec4f::MakeMask<1, 1, 0, 0>();
        break;
    }

    case Format::R32G32B32_Float:
    {
        const float* source = reinterpret_cast<const float*>(rowData) + 3u * (size_t)x;
        color = Vec4f(source) & Vec4f::MakeMask<1, 1, 1, 0>();
        break;
    }

    case Format::R32G32B32A32_Float:
    {
        const Vec4f* source = reinterpret_cast<const Vec4f*>(rowData) + (size_t)x;
        color = *source;
        break;
    }

    case Format::R16_Half:
    {
        const Half* source = reinterpret_cast<const Half*>(rowData) + (size_t)x;
        color = Vec4f(source->ToFloat());
        break;
    }

    case Format::R16G16_Half:
    {
        const Half2* source = reinterpret_cast<const Half2*>(rowData) + (size_t)x;
        color = Vec4f_Load_Half2(source);
        break;
    }

    case Format::R16G16B16_Half:
    {
        const Half3* source = reinterpret_cast<const Half3*>(rowData) + (size_t)x;
        color = Vec4f_Load_Half4(reinterpret_cast<const Half4*>(source)) & Vec4f::MakeMask<1, 1, 1, 0>();
        break;
    }

    case Format::R16G16B16A16_Half:
    {
        const Half4* source = reinterpret_cast<const Half4*>(rowData) + (size_t)x;
        color = Vec4f_Load_Half4(source);
        break;
    }

    case Format::R9G9B9E5_SharedExp:
    {
        const PackedUFloat3_9_9_9_5* source = reinterpret_cast<const PackedUFloat3_9_9_9_5*>(rowData);
        color = LoadVec4f(source[x]);
        break;
    }

    case Format::R11G11B10_Float:
    {
        const PackedUFloat3_11_11_10* source = reinterpret_cast<const PackedUFloat3_11_11_10*>(rowData);
        color = LoadVec4f(source[x]);
        break;
    }

    case Format::BC1:
    {
        color = DecodeBC1(reinterpret_cast<const uint8*>(mData), x, y, GetWidth());
        break;
    }

    case Format::BC4:
    {
        color = DecodeBC4(reinterpret_cast<const uint8*>(mData), x, y, GetWidth());
        break;
    }

    case Format::BC5:
    {
        color = DecodeBC5(reinterpret_cast<const uint8*>(mData), x, y, GetWidth());
        break;
    }

    default:
        NFE_FATAL("Unsupported bitmap format");
        color = Vec4f::Zero();
    }

    if (!mLinearSpace)
    {
        color = Convert_sRGB_To_Linear(color);
    }

    return color;
}

void Bitmap::GetPixelBlock(const Vec4ui coords, Vec4f* outColors) const
{
    const Vec4ui size2D = mSize.Swizzle<0,1,0,1>();
    NFE_ASSERT((coords < size2D).All(), "");

    const uint8* rowData0 = mData + GetStride() * static_cast<size_t>(coords.y);
    const uint8* rowData1 = mData + GetStride() * static_cast<size_t>(coords.w);

    Vec4f color[4];

    switch (mFormat)
    {
    case Format::R8_UNorm:
    {
        constexpr float scale = 1.0f / 255.0f;
        const uint32 value0 = rowData0[(uint32)coords.x];
        const uint32 value1 = rowData0[(uint32)coords.z];
        const uint32 value2 = rowData1[(uint32)coords.x];
        const uint32 value3 = rowData1[(uint32)coords.z];
        const Vec4f values = Vec4f::FromIntegers(value0, value1, value2, value3) * scale;
        color[0] = values.SplatX();
        color[1] = values.SplatY();
        color[2] = values.SplatZ();
        color[3] = values.SplatW();
        break;
    }

    case Format::R8G8_UNorm:
    {
        const Vec4ui offsets = coords << 1; // offset = 2 * coords
        color[0] = Vec4f_Load_2xUint8_Norm(rowData0 + offsets.x);
        color[1] = Vec4f_Load_2xUint8_Norm(rowData0 + offsets.z);
        color[2] = Vec4f_Load_2xUint8_Norm(rowData1 + offsets.x);
        color[3] = Vec4f_Load_2xUint8_Norm(rowData1 + offsets.z);
        break;
    }

    case Format::B8G8R8_UNorm:
    {
        const Vec4ui offsets = coords + (coords << 1); // offset = 3 * coords
        color[0] = Vec4f_LoadBGR_UNorm(rowData0 + offsets.x);
        color[1] = Vec4f_LoadBGR_UNorm(rowData0 + offsets.z);
        color[2] = Vec4f_LoadBGR_UNorm(rowData1 + offsets.x);
        color[3] = Vec4f_LoadBGR_UNorm(rowData1 + offsets.z);
        break;
    }

    case Format::B8G8R8A8_UNorm:
    {
        constexpr float scale = 1.0f / 255.0f;
        const Vec4ui offsets = coords << 2; // offset = 4 * coords
        color[0] = Vec4f_Load_4xUint8(rowData0 + offsets.x).Swizzle<2, 1, 0, 3>() * scale;
        color[1] = Vec4f_Load_4xUint8(rowData0 + offsets.z).Swizzle<2, 1, 0, 3>() * scale;
        color[2] = Vec4f_Load_4xUint8(rowData1 + offsets.x).Swizzle<2, 1, 0, 3>() * scale;
        color[3] = Vec4f_Load_4xUint8(rowData1 + offsets.z).Swizzle<2, 1, 0, 3>() * scale;
        break;
    }

    case Format::R8G8B8A8_UNorm:
    {
        constexpr float scale = 1.0f / 255.0f;
        const Vec4ui offsets = coords << 2; // offset = 4 * coords
        color[0] = Vec4f_Load_4xUint8(rowData0 + offsets.x) * scale;
        color[1] = Vec4f_Load_4xUint8(rowData0 + offsets.z) * scale;
        color[2] = Vec4f_Load_4xUint8(rowData1 + offsets.x) * scale;
        color[3] = Vec4f_Load_4xUint8(rowData1 + offsets.z) * scale;
        break;
    }

    case Format::B8G8R8A8_UNorm_Palette:
    {
        constexpr float scale = 1.0f / 255.0f;
        const uint8* source0 = mPalette + 4u * rowData0[coords.x];
        const uint8* source1 = mPalette + 4u * rowData0[coords.z];
        const uint8* source2 = mPalette + 4u * rowData1[coords.x];
        const uint8* source3 = mPalette + 4u * rowData1[coords.z];
        color[0] = Vec4f_Load_4xUint8(source0).Swizzle<2, 1, 0, 3>() * scale;
        color[1] = Vec4f_Load_4xUint8(source1).Swizzle<2, 1, 0, 3>() * scale;
        color[2] = Vec4f_Load_4xUint8(source2).Swizzle<2, 1, 0, 3>() * scale;
        color[3] = Vec4f_Load_4xUint8(source3).Swizzle<2, 1, 0, 3>() * scale;
        break;
    }

    case Format::B5G6R5_UNorm:
    {
        const Vec4ui offsets = coords << 1; // offset = 2 * coords
        color[0] = LoadVec4fUNorm(*reinterpret_cast<const Packed_5_6_5*>(rowData0 + offsets.x));
        color[1] = LoadVec4fUNorm(*reinterpret_cast<const Packed_5_6_5*>(rowData0 + offsets.z));
        color[2] = LoadVec4fUNorm(*reinterpret_cast<const Packed_5_6_5*>(rowData1 + offsets.x));
        color[3] = LoadVec4fUNorm(*reinterpret_cast<const Packed_5_6_5*>(rowData1 + offsets.z));
        break;
    }

    case Format::B4G4R4A4_UNorm:
    {
        const Vec4ui offsets = coords << 1; // offset = 2 * coords
        color[0] = LoadVec4fUNorm(*reinterpret_cast<const Packed_4_4_4_4*>(rowData0 + offsets.x));
        color[1] = LoadVec4fUNorm(*reinterpret_cast<const Packed_4_4_4_4*>(rowData0 + offsets.z));
        color[2] = LoadVec4fUNorm(*reinterpret_cast<const Packed_4_4_4_4*>(rowData1 + offsets.x));
        color[3] = LoadVec4fUNorm(*reinterpret_cast<const Packed_4_4_4_4*>(rowData1 + offsets.z));
        break;
    }

    case Format::R10G10B10A2_UNorm:
    {
        const Vec4ui offsets = coords << 2; // offset = 4 * coords
        color[0] = LoadVec4fUNorm(*reinterpret_cast<const Packed_10_10_10_2*>(rowData0 + offsets.x));
        color[1] = LoadVec4fUNorm(*reinterpret_cast<const Packed_10_10_10_2*>(rowData0 + offsets.z));
        color[2] = LoadVec4fUNorm(*reinterpret_cast<const Packed_10_10_10_2*>(rowData1 + offsets.x));
        color[3] = LoadVec4fUNorm(*reinterpret_cast<const Packed_10_10_10_2*>(rowData1 + offsets.z));
        break;
    }

    case Format::R16_UNorm:
    {
        constexpr float scale = 1.0f / 65535.0f;
        const uint32 value0 = reinterpret_cast<const uint16*>(rowData0)[(uint32)coords.x];
        const uint32 value1 = reinterpret_cast<const uint16*>(rowData0)[(uint32)coords.z];
        const uint32 value2 = reinterpret_cast<const uint16*>(rowData1)[(uint32)coords.x];
        const uint32 value3 = reinterpret_cast<const uint16*>(rowData1)[(uint32)coords.z];
        const Vec4f values = Vec4f::FromIntegers(value0, value1, value2, value3) * scale;
        color[0] = values.SplatX();
        color[1] = values.SplatY();
        color[2] = values.SplatZ();
        color[3] = values.SplatW();
        break;
    }

    case Format::R16G16_UNorm:
    {
        const Vec4ui offsets = coords << 2; // offset = 4 * coords
        const uint16* source0 = reinterpret_cast<const uint16*>(rowData0 + offsets.x);
        const uint16* source1 = reinterpret_cast<const uint16*>(rowData0 + offsets.z);
        const uint16* source2 = reinterpret_cast<const uint16*>(rowData1 + offsets.x);
        const uint16* source3 = reinterpret_cast<const uint16*>(rowData1 + offsets.z);
        color[0] = Vec4f_Load_2xUint16_Norm(source0);
        color[1] = Vec4f_Load_2xUint16_Norm(source1);
        color[2] = Vec4f_Load_2xUint16_Norm(source2);
        color[3] = Vec4f_Load_2xUint16_Norm(source3);
        break;
    }

    case Format::R16G16B16A16_UNorm:
    {
        constexpr float scale = 1.0f / 65535.0f;
        const Vec4ui offsets = coords << 3; // offset = 8 * coords
        const uint16* source0 = reinterpret_cast<const uint16*>(rowData0 + offsets.x);
        const uint16* source1 = reinterpret_cast<const uint16*>(rowData0 + offsets.z);
        const uint16* source2 = reinterpret_cast<const uint16*>(rowData1 + offsets.x);
        const uint16* source3 = reinterpret_cast<const uint16*>(rowData1 + offsets.z);
        color[0] = Vec4f_Load_4xUint16(source0) * scale;
        color[1] = Vec4f_Load_4xUint16(source1) * scale;
        color[2] = Vec4f_Load_4xUint16(source2) * scale;
        color[3] = Vec4f_Load_4xUint16(source3) * scale;
        break;
    }

    case Format::R32_Float:
    {
        const float* source0 = reinterpret_cast<const float*>(rowData0) + (uint32)coords.x;
        const float* source1 = reinterpret_cast<const float*>(rowData0) + (uint32)coords.z;
        const float* source2 = reinterpret_cast<const float*>(rowData1) + (uint32)coords.x;
        const float* source3 = reinterpret_cast<const float*>(rowData1) + (uint32)coords.z;
        color[0] = Vec4f(*source0);
        color[1] = Vec4f(*source1);
        color[2] = Vec4f(*source2);
        color[3] = Vec4f(*source3);
        break;
    }

    case Format::R32G32_Float:
    {
        const Vec4ui offsets = coords << 3; // offset = 8 * coords
        const float* source0 = reinterpret_cast<const float*>(rowData0 + offsets.x);
        const float* source1 = reinterpret_cast<const float*>(rowData0 + offsets.z);
        const float* source2 = reinterpret_cast<const float*>(rowData1 + offsets.x);
        const float* source3 = reinterpret_cast<const float*>(rowData1 + offsets.z);
        color[0] = Vec4f(source0) & Vec4f::MakeMask<1, 1, 0, 0>();
        color[1] = Vec4f(source1) & Vec4f::MakeMask<1, 1, 0, 0>();
        color[2] = Vec4f(source2) & Vec4f::MakeMask<1, 1, 0, 0>();
        color[3] = Vec4f(source3) & Vec4f::MakeMask<1, 1, 0, 0>();
        break;
    }

    case Format::R32G32B32_Float:
    {
        const Vec4ui offsets = coords + (coords << 1); // offset = 3 * coords
        const float* source0 = reinterpret_cast<const float*>(rowData0) + offsets.x;
        const float* source1 = reinterpret_cast<const float*>(rowData0) + offsets.z;
        const float* source2 = reinterpret_cast<const float*>(rowData1) + offsets.x;
        const float* source3 = reinterpret_cast<const float*>(rowData1) + offsets.z;
        color[0] = Vec4f(source0) & Vec4f::MakeMask<1, 1, 1, 0>();
        color[1] = Vec4f(source1) & Vec4f::MakeMask<1, 1, 1, 0>();
        color[2] = Vec4f(source2) & Vec4f::MakeMask<1, 1, 1, 0>();
        color[3] = Vec4f(source3) & Vec4f::MakeMask<1, 1, 1, 0>();
        break;
    }

    case Format::R32G32B32A32_Float:
    {
        color[0] = reinterpret_cast<const Vec4f*>(rowData0)[coords.x];
        color[1] = reinterpret_cast<const Vec4f*>(rowData0)[coords.z];
        color[2] = reinterpret_cast<const Vec4f*>(rowData1)[coords.x];
        color[3] = reinterpret_cast<const Vec4f*>(rowData1)[coords.z];
        break;
    }

    case Format::R16_Half:
    {
        const Vec4ui offsets = coords << 1; // offset = 2 * coords
        const Half* source0 = reinterpret_cast<const Half*>(rowData0 + offsets.x);
        const Half* source1 = reinterpret_cast<const Half*>(rowData0 + offsets.z);
        const Half* source2 = reinterpret_cast<const Half*>(rowData1 + offsets.x);
        const Half* source3 = reinterpret_cast<const Half*>(rowData1 + offsets.z);
        color[0] = Vec4f(source0->ToFloat());
        color[1] = Vec4f(source1->ToFloat());
        color[2] = Vec4f(source2->ToFloat());
        color[3] = Vec4f(source3->ToFloat());
        break;
    }

    case Format::R16G16_Half:
    {
        const Vec4ui offsets = coords << 2; // offset = 4 * coords
        const Half* source0 = reinterpret_cast<const Half*>(rowData0 + offsets.x);
        const Half* source1 = reinterpret_cast<const Half*>(rowData0 + offsets.z);
        const Half* source2 = reinterpret_cast<const Half*>(rowData1 + offsets.x);
        const Half* source3 = reinterpret_cast<const Half*>(rowData1 + offsets.z);
        color[0] = Vec4f_Load_Half2(reinterpret_cast<const Half2*>(source0));
        color[1] = Vec4f_Load_Half2(reinterpret_cast<const Half2*>(source1));
        color[2] = Vec4f_Load_Half2(reinterpret_cast<const Half2*>(source2));
        color[3] = Vec4f_Load_Half2(reinterpret_cast<const Half2*>(source3));
        break;
    }

    case Format::R16G16B16_Half:
    {
        const Vec4ui offsets = (coords << 2) + (coords << 1); // offset = 6 * coords
        const Half* source0 = reinterpret_cast<const Half*>(rowData0 + offsets.x);
        const Half* source1 = reinterpret_cast<const Half*>(rowData0 + offsets.z);
        const Half* source2 = reinterpret_cast<const Half*>(rowData1 + offsets.x);
        const Half* source3 = reinterpret_cast<const Half*>(rowData1 + offsets.z);
        color[0] = Vec4f_Load_Half4(reinterpret_cast<const Half4*>(source0)) & Vec4f::MakeMask<1, 1, 1, 0>();
        color[1] = Vec4f_Load_Half4(reinterpret_cast<const Half4*>(source1)) & Vec4f::MakeMask<1, 1, 1, 0>();
        color[2] = Vec4f_Load_Half4(reinterpret_cast<const Half4*>(source2)) & Vec4f::MakeMask<1, 1, 1, 0>();
        color[3] = Vec4f_Load_Half4(reinterpret_cast<const Half4*>(source3)) & Vec4f::MakeMask<1, 1, 1, 0>();
        break;
    }

    case Format::R16G16B16A16_Half:
    {
        const Vec4ui offsets = coords << 3; // offset = 8 * coords
        const Half* source0 = reinterpret_cast<const Half*>(rowData0 + offsets.x);
        const Half* source1 = reinterpret_cast<const Half*>(rowData0 + offsets.z);
        const Half* source2 = reinterpret_cast<const Half*>(rowData1 + offsets.x);
        const Half* source3 = reinterpret_cast<const Half*>(rowData1 + offsets.z);
        color[0] = Vec4f_Load_Half4(reinterpret_cast<const Half4*>(source0));
        color[1] = Vec4f_Load_Half4(reinterpret_cast<const Half4*>(source1));
        color[2] = Vec4f_Load_Half4(reinterpret_cast<const Half4*>(source2));
        color[3] = Vec4f_Load_Half4(reinterpret_cast<const Half4*>(source3));
        break;
    }
    
    case Format::R9G9B9E5_SharedExp:
    {
        const Vec4ui offsets = coords << 2; // offset = 4 * coords
        color[0] = LoadVec4f(*reinterpret_cast<const PackedUFloat3_9_9_9_5*>(rowData0 + offsets.x));
        color[1] = LoadVec4f(*reinterpret_cast<const PackedUFloat3_9_9_9_5*>(rowData0 + offsets.z));
        color[2] = LoadVec4f(*reinterpret_cast<const PackedUFloat3_9_9_9_5*>(rowData1 + offsets.x));
        color[3] = LoadVec4f(*reinterpret_cast<const PackedUFloat3_9_9_9_5*>(rowData1 + offsets.z));
        break;
    }

    case Format::R11G11B10_Float:
    {
        const Vec4ui offsets = coords << 2; // offset = 4 * coords
        color[0] = LoadVec4f(*reinterpret_cast<const PackedUFloat3_11_11_10*>(rowData0 + offsets.x));
        color[1] = LoadVec4f(*reinterpret_cast<const PackedUFloat3_11_11_10*>(rowData0 + offsets.z));
        color[2] = LoadVec4f(*reinterpret_cast<const PackedUFloat3_11_11_10*>(rowData1 + offsets.x));
        color[3] = LoadVec4f(*reinterpret_cast<const PackedUFloat3_11_11_10*>(rowData1 + offsets.z));
        break;
    }

    case Format::BC1:
    {
        color[0] = DecodeBC1(mData, coords.x, coords.y, GetWidth());
        color[1] = DecodeBC1(mData, coords.z, coords.y, GetWidth());
        color[2] = DecodeBC1(mData, coords.x, coords.w, GetWidth());
        color[3] = DecodeBC1(mData, coords.z, coords.w, GetWidth());
        break;
    }

    case Format::BC4:
    {
        color[0] = DecodeBC4(mData, coords.x, coords.y, GetWidth());
        color[1] = DecodeBC4(mData, coords.z, coords.y, GetWidth());
        color[2] = DecodeBC4(mData, coords.x, coords.w, GetWidth());
        color[3] = DecodeBC4(mData, coords.z, coords.w, GetWidth());
        break;
    }

    case Format::BC5:
    {
        color[0] = DecodeBC5(mData, coords.x, coords.y, GetWidth());
        color[1] = DecodeBC5(mData, coords.z, coords.y, GetWidth());
        color[2] = DecodeBC5(mData, coords.x, coords.w, GetWidth());
        color[3] = DecodeBC5(mData, coords.z, coords.w, GetWidth());
        break;
    }

    default:
        NFE_FATAL("Unsupported bitmap format");
    }

    if (!mLinearSpace)
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

const Vec4f Bitmap::GetPixel3D(uint32 x, uint32 y, uint32 z) const
{
    NFE_ASSERT(x < GetWidth(), "");
    NFE_ASSERT(y < GetHeight(), "");
    NFE_ASSERT(z < GetDepth(), "");

    const size_t row = y + static_cast<size_t>(GetHeight()) * static_cast<size_t>(z);
    const uint8* rowData = mData + GetStride() * row;

    Vec4f color;
    switch (mFormat)
    {
    case Format::R8_UNorm:
    {
        const uint32 value = rowData[x];
        color = Vec4f::FromInteger(value) * (1.0f / 255.0f);
        break;
    }
    case Format::R32_Float:
    {
        const float* source = reinterpret_cast<const float*>(rowData) + (size_t)x;
        color = Vec4f(*source);
        break;
    }

    // TODO more formats

    default:
        NFE_FATAL("Unsupported bitmap format");
        color = Vec4f::Zero();
    }

    if (!mLinearSpace)
    {
        color = Convert_sRGB_To_Linear(color);
    }

    return color;
}

void Bitmap::GetPixelBlock3D(const Vec4ui coordsA, const Vec4ui coordsB, Vec4f* outColors) const
{
    NFE_ASSERT((coordsA < mSize).All3(), "");
    NFE_ASSERT((coordsB < mSize).All3(), "");

    const size_t row0 = coordsA.y + static_cast<size_t>(GetHeight()) * static_cast<size_t>(coordsA.z);
    const size_t row1 = coordsB.y + static_cast<size_t>(GetHeight()) * static_cast<size_t>(coordsA.z);
    const size_t row2 = coordsA.y + static_cast<size_t>(GetHeight()) * static_cast<size_t>(coordsB.z);
    const size_t row3 = coordsB.y + static_cast<size_t>(GetHeight()) * static_cast<size_t>(coordsB.z);

    const uint8* rowData0 = mData + GetStride() * row0;
    const uint8* rowData1 = mData + GetStride() * row1;
    const uint8* rowData2 = mData + GetStride() * row2;
    const uint8* rowData3 = mData + GetStride() * row3;

    Vec4f color[8];

    switch (mFormat)
    {
    case Format::R8_UNorm:
    {
        color[0] = Vec4f::FromInteger(static_cast<uint32>(rowData0[coordsA.x]));
        color[1] = Vec4f::FromInteger(static_cast<uint32>(rowData0[coordsB.x]));
        color[2] = Vec4f::FromInteger(static_cast<uint32>(rowData1[coordsA.x]));
        color[3] = Vec4f::FromInteger(static_cast<uint32>(rowData1[coordsB.x]));
        color[4] = Vec4f::FromInteger(static_cast<uint32>(rowData2[coordsA.x]));
        color[5] = Vec4f::FromInteger(static_cast<uint32>(rowData2[coordsB.x]));
        color[6] = Vec4f::FromInteger(static_cast<uint32>(rowData3[coordsA.x]));
        color[7] = Vec4f::FromInteger(static_cast<uint32>(rowData3[coordsB.x]));
        break;
    }
    case Format::R16_UNorm:
    {
        const Vec4f scale(1.0f / 65535.0f);
        color[0] = Vec4f::FromInteger(reinterpret_cast<const uint16*>(rowData0)[coordsA.x]);
        color[1] = Vec4f::FromInteger(reinterpret_cast<const uint16*>(rowData0)[coordsB.x]);
        color[2] = Vec4f::FromInteger(reinterpret_cast<const uint16*>(rowData1)[coordsA.x]);
        color[3] = Vec4f::FromInteger(reinterpret_cast<const uint16*>(rowData1)[coordsB.x]);
        color[4] = Vec4f::FromInteger(reinterpret_cast<const uint16*>(rowData2)[coordsA.x]);
        color[5] = Vec4f::FromInteger(reinterpret_cast<const uint16*>(rowData2)[coordsB.x]);
        color[6] = Vec4f::FromInteger(reinterpret_cast<const uint16*>(rowData3)[coordsA.x]);
        color[7] = Vec4f::FromInteger(reinterpret_cast<const uint16*>(rowData3)[coordsB.x]);
        break;
    }
    case Format::R32_Float:
    {
        color[0] = Vec4f(reinterpret_cast<const float*>(rowData0)[coordsA.x]);
        color[1] = Vec4f(reinterpret_cast<const float*>(rowData0)[coordsB.x]);
        color[2] = Vec4f(reinterpret_cast<const float*>(rowData1)[coordsA.x]);
        color[3] = Vec4f(reinterpret_cast<const float*>(rowData1)[coordsB.x]);
        color[4] = Vec4f(reinterpret_cast<const float*>(rowData2)[coordsA.x]);
        color[5] = Vec4f(reinterpret_cast<const float*>(rowData2)[coordsB.x]);
        color[6] = Vec4f(reinterpret_cast<const float*>(rowData3)[coordsA.x]);
        color[7] = Vec4f(reinterpret_cast<const float*>(rowData3)[coordsB.x]);
        break;
    }
    // TODO more formats

    default:
        NFE_FATAL("Unsupported bitmap format");
    }

    if (!mLinearSpace)
    {
        for (size_t i = 0; i < 8u; ++i)
        {
            color[i] = Convert_sRGB_To_Linear(color[i]);
        }
    }

    for (size_t i = 0; i < 8u; ++i)
    {
        outColors[i] = color[i];
    }
}

bool Bitmap::Scale(const Math::Vec4f& factor)
{
    const uint32 width = GetWidth();
    const uint32 height = GetHeight();

    if (mFormat == Format::R32G32B32_Float)
    {
        for (uint32 y = 0; y < height; ++y)
        {
            for (uint32 x = 0; x < width; ++x)
            {
                Vec3f& pixelPtr = GetPixelRef<Vec3f>(x, y);
                pixelPtr.x *= factor.x;
                pixelPtr.y *= factor.y;
                pixelPtr.z *= factor.z;
            }
        }
        return true;
    }
    else if (mFormat == Format::R32G32B32A32_Float)
    {
        for (uint32 y = 0; y < height; ++y)
        {
            for (uint32 x = 0; x < width; ++x)
            {
                Vec4f& pixelPtr = GetPixelRef<Vec4f>(x, y);
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

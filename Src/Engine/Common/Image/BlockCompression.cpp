#include "PCH.hpp"
#include "../Common/Math/Vec4i.hpp"

namespace NFE {

using namespace Math;

namespace helper
{
    static Vec4f DecodeBC_4ColorPalette(const uint8* blockData, const uint32 x, const uint32 y)
    {
        // extract base colors for given block
        const Vec4i mask = { 0x1F << 11, 0x3F << 5, 0x1F, 0 };
        const Vec4i raw0 = Vec4i(*reinterpret_cast<const int32*>(blockData + 0)) & mask;
        const Vec4i raw1 = Vec4i(*reinterpret_cast<const int32*>(blockData + 2)) & mask;
        const Vec4f color0 = raw0.ConvertToVec4f();
        const Vec4f color1 = raw1.ConvertToVec4f();

        const uint32 code = *reinterpret_cast<const uint32*>(blockData + 4);
        const uint32 codeOffset = 2 * (4 * y + x);
        const uint32 index = (code >> codeOffset) & 0b11;

        // calculate final color by blending base colors + scale down from 5,6,5 bit ranges to 0...1 float range
        const float weights[] = { 0.0f, 1.0f, 1.0f / 3.0f, 2.0f / 3.0f };
        const Vec4f scale{ 1.0f / 2048.0f / 31.0f, 1.0f / 32.0f / 63.0f, 1.0f / 31.0f, 0.0f };
        return Vec4f::Lerp(color0, color1, weights[index]) * scale;
    }

    static bool DecodeBC_4ColorPalette_Block(const uint8* blockData, uint32 width, uint32 blockX, uint32 blockY, float* outData)
    {
        // extract base colors for given block
        const Vec4i mask = { 0x1F << 11, 0x3F << 5, 0x1F, 0 };
        const Vec4i raw0 = Vec4i(*reinterpret_cast<const int32*>(blockData + 0)) & mask;
        const Vec4i raw1 = Vec4i(*reinterpret_cast<const int32*>(blockData + 2)) & mask;
        const Vec4f color0 = raw0.ConvertToVec4f();
        const Vec4f color1 = raw1.ConvertToVec4f();

        const float weights[] = { 0.0f, 1.0f, 1.0f / 3.0f, 2.0f / 3.0f };
        const Vec4f scale{ 1.0f / 2048.0f / 31.0f, 1.0f / 32.0f / 63.0f, 1.0f / 31.0f, 0.0f };

        uint32 code = *reinterpret_cast<const uint32*>(blockData + 4);
        for (int idx = 15; idx >= 0; --idx)
        {
            const int y = idx / 4;
            const int x = idx % 4;
            const uint32 index = code & 0b11;
            code >>= 2;
            const Vec4f pixelColor = Vec4f::Lerp(color0, color1, weights[index]) * scale;
            const size_t dataIdx = 4 * (width * (4ull * blockY + y) + 4ull * blockX + x);
            outData[dataIdx] = pixelColor[0];
            outData[dataIdx + 1] = pixelColor[1];
            outData[dataIdx + 2] = pixelColor[2];
            outData[dataIdx + 3] = 1.0f;
        }

        return true;
    }

    static Vec4f DecodeBC_7ColorPalette(const uint8* blockData, const uint32 x, const uint32 y, const float minVal = 0.f, const float maxVal = 1.f)
    {
        const uint8 intColor0 = blockData[0];
        const uint8 intColor1 = blockData[1];
        const Vec4f color0(static_cast<float>(intColor0) / 255.0f);
        const Vec4f color1(static_cast<float>(intColor1) / 255.0f);

        const uint64 code = *reinterpret_cast<const uint64*>(blockData + 2);
        const uint32 codeOffset = 3 * (4 * y + x);
        const uint32 index = (code >> codeOffset) % 8;

        const float weights7[] = { 0.0f, 1.0f, 1.0f / 7.f, 2.0f / 7.f, 3.0f / 7.f, 4.0f / 7.f, 5.0f / 7.f, 6.0f / 7.f };
        const float weights5[] = { 0.0f, 1.0f, 1.0f / 5.f, 2.0f / 5.f, 3.0f / 5.f, 4.0f / 5.f };
        const Vec4f scale{ 1.0f / 31.0f, 1.0f / 63.0f, 1.0f / 31.0f, 0.0f };

        if (intColor0 > intColor1)
        {
            return Vec4f::Lerp(color0, color1, weights7[index]) * scale;
        }
        else if (index == 6)
        {
            return Vec4f(minVal);
        }
        else if (index == 7)
        {
            return Vec4f(maxVal);
        }
        else
        {
            return Vec4f::Lerp(color0, color1, weights5[index]) * scale;
        }
    }

} // helper


NFE_API_EXPORT
NFE_FORCE_NOINLINE
const Vec4f DecodeBC1(const uint8* data, uint32 x, uint32 y, const uint32 width)
{
    const size_t blocksInRow = width / 4u; // TODO non-4-multiply width support
    const size_t blockX = x / 4u;
    const size_t blockY = y / 4u;

    // calculate position inside block
    x %= 4u;
    y %= 4u;

    const uint8* blockData = data + 8u * (blocksInRow * blockY + blockX);
    const Vec4f alpha(0.0f, 0.0f, 0.0f, 1.0f);
    return helper::DecodeBC_4ColorPalette(blockData, x, y) + alpha;
}

NFE_API_EXPORT
NFE_FORCE_NOINLINE
std::unique_ptr<float[]> DecodeBC1Image(const uint8* data, const uint32 w, const uint32 h)
{
    const size_t blocksInRow = w / 4u; // TODO non-4-multiply width support
    const size_t rows = h / 4u;

    const size_t decodedSize = w * h * 4ull;
    std::unique_ptr<float[]> decodedData = std::make_unique<float[]>(decodedSize);

    for (uint32 blockY = 0; blockY < rows; ++blockY)
    {
        for (uint32 blockX = 0; blockX < blocksInRow; ++blockX)
        {
            const uint8* blockData = data + 8u * (blocksInRow * blockY + blockX);
            helper::DecodeBC_4ColorPalette_Block(blockData, w, blockX, blockY, decodedData.get());
        }
    }

    return decodedData;
}

NFE_API_EXPORT
NFE_FORCE_NOINLINE
const Vec4f DecodeBC2(const uint8* data, uint32 x, uint32 y, const uint32 width)
{
    const size_t blocksInRow = width / 4u; // TODO non-4-multiply width support
    const size_t blockX = x / 4u;
    const size_t blockY = y / 4u;

    // calculate position inside block
    x %= 4u;
    y %= 4u;

    const uint8* blockData = data + 16u * (blocksInRow * blockY + blockX);

    const uint64 alphaData = *reinterpret_cast<const uint64*>(blockData);
    const uint32 alphaOffset = 4u * (4u * y + x);
    const float alphaValue = ((alphaData >> alphaOffset) & 0b1111) / 15.0f;

    const Vec4f alpha(0.0f, 0.0f, 0.0f, alphaValue);
    return helper::DecodeBC_4ColorPalette(blockData + 8, x, y) + alpha;
}

NFE_API_EXPORT
NFE_FORCE_NOINLINE
const Vec4f DecodeBC3(const uint8* data, uint32 x, uint32 y, const uint32 width)
{
    const size_t blocksInRow = width / 4u; // TODO non-4-multiply width support
    const size_t blockX = x / 4u;
    const size_t blockY = y / 4u;

    // calculate position inside block
    x %= 4u;
    y %= 4u;

    const uint8* blockData = data + 16u * (blocksInRow * blockY + blockX);

    const Vec4f alpha = helper::DecodeBC_7ColorPalette(blockData, x, y);
    const Vec4f color = helper::DecodeBC_4ColorPalette(blockData + 8, x, y);

    return color + alpha.Swizzle(3, 3, 3, 0);
}

NFE_FORCE_NOINLINE
const Vec4f DecodeBC4(const uint8* data, uint32 x, uint32 y, const uint32 width)
{
    const size_t blocksInRow = width / 4; // TODO non-4-multiply width support
    const size_t blockX = x / 4u;
    const size_t blockY = y / 4u;

    // calculate position inside block
    x %= 4;
    y %= 4;

    const uint8* blockData = reinterpret_cast<const uint8*>(data + 8 * (blocksInRow * blockY + blockX));
    Vec4f alpha(0.0f, 0.0f, 0.0f, 1.0f);
    return helper::DecodeBC_7ColorPalette(blockData, x, y) + alpha;
}

NFE_FORCE_NOINLINE
const Vec4f DecodeBC5(const uint8* data, uint32 x, uint32 y, const uint32 width)
{
    const size_t blocksInRow = width / 4; // TODO non-4-multiply width support
    const size_t blockX = x / 4u;
    const size_t blockY = y / 4u;

    // calculate position inside block
    x %= 4;
    y %= 4;

    const uint8* blockDataRed = reinterpret_cast<const uint8*>(data + 16 * (blocksInRow * blockY + blockX));
    const Vec4f maskRed = { 0xFFFF, 0, 0, 0 };
    Vec4f red(0.0f, 0.0f, 0.0f, 1.0f);
    red += helper::DecodeBC_7ColorPalette(blockDataRed, x, y) & maskRed;

    const uint8* blockDataGreen = blockDataRed + 8;
    const Vec4f green = helper::DecodeBC_7ColorPalette(blockDataGreen, x, y);

    return Vec4f::Select(red, green, { 0, 1, 0, 0});
}

} // namespace NFE

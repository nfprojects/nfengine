#include "PCH.h"
#include "GenericSampler.h"
#include "../nfCommon/Math/Random.hpp"
#include "../nfCommon/Logger/Logger.hpp"

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

// TODO move to Common
namespace BlueNoise
{

static const char* FilePath = "../Data/BlueNoise128_RGBA16.dat";
static constexpr uint32 TextureLayers = 4;
static constexpr uint32 TextureBits = 16;
static constexpr uint32 TextureSize = 128;

static const uint16* LoadTexture()
{
    FILE* file = fopen(FilePath, "rb");
    if (!file)
    {
        NFE_LOG_ERROR("Failed to load blue noise texture");
        return nullptr;
    }

    const size_t dataSize = (TextureBits / 8) * TextureLayers * TextureSize * TextureSize;
    uint16* data = (uint16*)NFE_MALLOC(dataSize, NFE_CACHE_LINE_SIZE);
    if (!data)
    {
        fclose(file);
        NFE_LOG_ERROR("Failed to allocate memory for blue noise texture");
        return nullptr;
    }

    if (fread(data, dataSize, 1, file) != 1)
    {
        fclose(file);
        NFE_LOG_ERROR("Failed to read blue noise texture");
        return nullptr;
    }

    NFE_LOG_INFO("Blue noise texture loaded successfully");
    fclose(file);
    return data;
}

static const uint16* GetTexture()
{
    static const uint16* texture = LoadTexture();
    return texture;
}

} // BlueNoise

NFE_FORCE_INLINE static uint32 XorShift(uint32 x)
{
    x ^= x << 13u;
    x ^= x >> 17u;
    x ^= x << 5u;
    return x;
}

GenericSampler::GenericSampler()
    : mBlueNoiseTexture(BlueNoise::GetTexture())
{
}

void GenericSampler::ResetFrame(const DynArray<uint32>& seed, bool useBlueNoise)
{
    mCurrentSample = seed;
    mBlueNoiseTextureLayers = mBlueNoiseTexture && useBlueNoise ? BlueNoise::TextureLayers : 0;
}

void GenericSampler::ResetPixel(const uint32 x, const uint32 y)
{
    mBlueNoisePixelX = x & (BlueNoise::TextureSize - 1u);
    mBlueNoisePixelY = y & (BlueNoise::TextureSize - 1u);
    mSalt = (uint32)Hash((uint64)(x | (y << 16)));
    mSamplesGenerated = 0;
}

uint32 GenericSampler::GetInt()
{
    uint32 sample;

    if (mSamplesGenerated < mCurrentSample.Size())
    {
        sample = mCurrentSample[mSamplesGenerated];

        if (mSamplesGenerated < mBlueNoiseTextureLayers) // blue noise dithering
        {
            const uint32 pixelIndex = BlueNoise::TextureSize * mBlueNoisePixelY + mBlueNoisePixelX;
            const uint16* blueNoiseData = mBlueNoiseTexture + BlueNoise::TextureLayers * pixelIndex;
            sample += static_cast<uint32>(blueNoiseData[mSamplesGenerated]) << 16;
        }
        else
        {
            uint32 salt = mSalt;
            mSalt = XorShift(salt);
            sample += salt;
        }

        mSamplesGenerated++;
    }
    else // fallback to uniform random sampling
    {
        sample = fallbackGenerator->GetInt();
    }

    return sample;
}

} // namespace RT
} // namespace NFE

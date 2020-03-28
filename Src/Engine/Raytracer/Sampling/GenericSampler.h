#pragma once

#include "../Raytracer.h"
#include "../../Common/Math/Vec3f.hpp"
#include "../../Common/Containers/DynArray.hpp"

namespace NFE {
namespace RT {

class GenericSampler
{
public:
    GenericSampler();
    ~GenericSampler() = default;

    // move to next frame
    void ResetFrame(const Common::DynArray<uint32>& sample, bool useBlueNoise);

    // move to next pixel
    void ResetPixel(const uint32 x, const uint32 y);

    // get next sample
    // NOTE: effectively goes to next sample dimension
    NFE_RAYTRACER_API uint32 GetInt();

    NFE_FORCE_INLINE float GetFloat()
    {
        return Math::Min(0.999999940395f, static_cast<float>(GetInt()) / 4294967296.0f);
    }

    NFE_FORCE_INLINE const Math::Vec2f GetVec2f()
    {
        return Math::Vec2f{ GetFloat(), GetFloat() };
    }

    NFE_FORCE_INLINE const Math::Vec3f GetVec3f()
    {
        return Math::Vec3f{ GetFloat(), GetFloat(), GetFloat() };
    }

    Math::Random* fallbackGenerator = nullptr;

private:

    uint32 mBlueNoisePixelX = 0;
    uint32 mBlueNoisePixelY = 0;
    uint32 mBlueNoiseTextureLayers = 0;

    uint32 mSalt = 0;
    uint32 mSamplesGenerated = 0;

    const uint16* mBlueNoiseTexture = nullptr;

    Common::DynArray<uint32> mCurrentSample;
};


} // namespace RT
} // namespace NFE

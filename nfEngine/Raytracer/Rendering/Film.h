#pragma once

#include "../Raytracer.h"
#include "../../nfCommon/Math/Vector4.hpp"

namespace NFE {
namespace RT {

class Film
{
public:
    NFE_RAYTRACER_API Film();
    NFE_RAYTRACER_API Film(Bitmap& sum, Bitmap* secondarySum = nullptr);

    NFE_FORCE_INLINE uint32 GetWidth() const
    {
        return mWidth;
    }

    NFE_FORCE_INLINE uint32 GetHeight() const
    {
        return mHeight;
    }

    void AccumulateColor(const Math::Vector4& pos, const Math::Vector4& sampleColor, Math::Random& randomGenerator);
    void AccumulateColor(const uint32 x, const uint32 y, const Math::Vector4& sampleColor);

private:
    Math::Vector4 mFilmSize;

    Bitmap* mSum;
    Bitmap* mSecondarySum;

    const uint32 mWidth;
    const uint32 mHeight;
};

} // namespace RT
} // namespace NFE

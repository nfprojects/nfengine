#pragma once

#include "Math.hpp"

namespace NFE {
namespace Math {
namespace WindowFunctions {


NFE_INLINE float MitchellNetravali(const float x, const float B, const float C)
{
    const float ax = Abs(x);
    const float ax2 = ax * ax;
    const float ax3 = ax * ax2;

    if (ax < 1.0f)
    {
        return ((12.0f - 9.0f * B - 6.0f * C) * ax3 + (-18.0f + 12.0f * B + 6.0f * C) * ax2 + (6.0f - 2.0f * B)) / 6.0f;
    }
    else if (ax < 2.0f)
    {
        return ((-B - 6.0f * C) * ax3 + (6.0f * B + 30.0f * C) * ax2 + (-12.0f * B - 48.0f * C) * ax + (8.0f * B + 24.0f * C)) / 6.0f;
    }
    else
    {
        return 0.0f;
    }
}

NFE_FORCE_INLINE float CatmullRom(const float x)
{
    return MitchellNetravali(x, 0.0f, 0.5f);
}

} // namespace WindowFunctions
} // namespace Math
} // namespace NFE

#pragma once

#include "../Raytracer.h"

#ifdef NFE_ENABLE_SPECTRAL_RENDERING
#include "../../Common/Math/Vec8f.hpp"
#else
#include "../../Common/Math/Vec4f.hpp"
#endif

namespace NFE {
namespace RT {

// Represents ray wavelength(s), randomized for primary rays
struct Wavelength
{
    static constexpr float Lower = 0.380e-6f;
    static constexpr float Higher = 0.720e-6f;

#ifdef NFE_ENABLE_SPECTRAL_RENDERING

#ifdef NFE_USE_AVX
    static constexpr uint32 NumComponents = 8;
    using ValueType = Math::Vec8f;
#else
    static constexpr uint32 NumComponents = 4;
    using ValueType = Math::Vec4f;
#endif // NFE_USE_AVX

    // sample tabularized data and return ray color
    const ValueType SampleSpectrum(const float* data, const uint32 numValues) const;

#else // !defined(NFE_ENABLE_SPECTRAL_RENDERING)

    static constexpr uint32 NumComponents = 3;
    using ValueType = Math::Vec4f;

#endif // NFE_ENABLE_SPECTRAL_RENDERING

    ValueType value;

#ifdef NFE_ENABLE_SPECTRAL_RENDERING
    bool isSingle = false;
#endif

    NFE_RAYTRACER_API void InitRange(uint32 binId, uint32 numBins);
    NFE_RAYTRACER_API void Randomize(float u);

    NFE_FORCE_INLINE float GetBase() const
    {
        return value[0];
    }

    NFE_FORCE_NOINLINE const ValueType GetInMeters() const
    {
        return ValueType(Lower) + value * (Higher - Lower);
    }
};


} // namespace RT
} // namespace NFE

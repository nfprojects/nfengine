#pragma once

#include "../Raytracer.h"
#include "../Config.h"

#ifdef NFE_ENABLE_SPECTRAL_RENDERING
#include "../../nfCommon/Math/Vector8.hpp"
#else
#include "../../nfCommon/Math/Vector4.hpp"
#endif

namespace NFE {
namespace RT {

// Represents ray wavelength(s), randomized for primary rays
struct Wavelength
{
    static constexpr float Lower = 0.380e-6f;
    static constexpr float Higher = 0.720e-6f;

#ifdef NFE_ENABLE_SPECTRAL_RENDERING
    static constexpr uint32 NumComponents = 8;
    using ValueType = Math::Vector8;
#else
    static constexpr uint32 NumComponents = 4;
    using ValueType = Math::Vector4;
#endif

    ValueType value;

#ifdef NFE_ENABLE_SPECTRAL_RENDERING
    bool isSingle = false;
#endif

    NFE_RAYTRACER_API void Randomize(float u);

    NFE_FORCE_INLINE float GetBase() const
    {
        return value[0];
    }
};


} // namespace RT
} // namespace NFE

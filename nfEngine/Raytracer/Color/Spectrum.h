#pragma once

#include "../Raytracer.h"
#include "../../nfCommon/Math/Vector4.hpp"

namespace NFE {
namespace RT {

// Represents spectral power distribution (SPD)
struct Spectrum
{
    Math::Vector4 rgbValues;

    NFE_FORCE_INLINE Spectrum() = default;
    NFE_FORCE_INLINE Spectrum(const Math::Vector4& rgbValues) : rgbValues(rgbValues) { }
};


} // namespace RT
} // namespace NFE

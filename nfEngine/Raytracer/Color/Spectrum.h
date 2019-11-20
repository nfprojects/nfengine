#pragma once

#include "../Raytracer.h"
#include "../../nfCommon/Math/Vector4.hpp"
#include "../../nfCommon/Math/Vector4Load.hpp"
#include "../../nfCommon/Math/LdrColor.hpp"
#include "../../nfCommon/Math/HdrColor.hpp"
#include "../../nfCommon/Math/ColorHelpers.hpp"

namespace NFE {
namespace RT {

// Represents spectral power distribution (SPD)
struct Spectrum
{
    Math::Vector4 rgbValues;

    NFE_FORCE_INLINE Spectrum() = default;
    NFE_FORCE_INLINE Spectrum(const Math::Vector4& rgbValues) : rgbValues(rgbValues) { }
    NFE_FORCE_INLINE Spectrum(const Math::LdrColorRGB& color)
    {
        rgbValues = Math::Vector4_LoadRGB_UNorm(&color.r);
    }

    NFE_FORCE_INLINE Spectrum(const Math::HdrColorRGB& color)
    {
        rgbValues = color.ToVector4();
    }

    NFE_FORCE_INLINE static const Spectrum One()
    {
        return Math::Vector4(1.0f, 1.0f, 1.0f);
    }

    NFE_FORCE_INLINE bool IsValid() const
    {
        return rgbValues.IsValid();
    }

    NFE_FORCE_INLINE bool IsZero() const
    {
        return rgbValues.IsZero().All();
    }

    NFE_FORCE_INLINE float Luminance() const
    {
        return Math::Vector4::Dot3(rgbValues, Math::c_rgbIntensityWeights);
    }

    NFE_FORCE_INLINE float Max() const
    {
        return Math::Max(rgbValues.x, Math::Max(rgbValues.y, rgbValues.z));
    }
};


} // namespace RT
} // namespace NFE

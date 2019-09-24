#pragma once

#include "Wavelength.h"
#include "Spectrum.h"

namespace NFE {
namespace RT {


// Represents a ray color/weight during raytracing
// The color values corresponds to wavelength values.
struct RayColor
{
    Wavelength::ValueType value;

    NFE_FORCE_INLINE RayColor() = default;
    NFE_FORCE_INLINE RayColor(const RayColor& other) = default;
    NFE_FORCE_INLINE RayColor& operator = (const RayColor& other) = default;

    NFE_FORCE_INLINE explicit RayColor(const float val) : value(val) { }
    NFE_FORCE_INLINE explicit RayColor(const Wavelength::ValueType& val) : value(val) { }

    NFE_FORCE_INLINE static const RayColor Zero()
    {
        return RayColor{ Wavelength::ValueType::Zero() };
    }

    NFE_FORCE_INLINE static const RayColor One()
    {
#ifdef NFE_ENABLE_SPECTRAL_RENDERING
        return RayColor{ Math::VECTOR8_ONE };
#else
        return RayColor{ Math::VECTOR_ONE };
#endif
    }

    NFE_FORCE_INLINE static const RayColor SingleWavelengthFallback()
    {
#ifdef NFE_ENABLE_SPECTRAL_RENDERING
        return RayColor{ Wavelength::ValueType((float)Wavelength::NumComponents, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f) };
#else
        return RayColor{ Math::VECTOR_ONE };
#endif
    }

    NFE_FORCE_INLINE const RayColor operator + (const RayColor& other) const
    {
        return RayColor{ value + other.value };
    }

    NFE_FORCE_INLINE const RayColor operator * (const RayColor& other) const
    {
        return RayColor{ value * other.value };
    }

    NFE_FORCE_INLINE const RayColor operator / (const RayColor& other) const
    {
        return RayColor{ value / other.value };
    }

    NFE_FORCE_INLINE const RayColor operator * (const float factor) const
    {
        return RayColor{ value * factor };
    }

    NFE_FORCE_INLINE const RayColor operator / (const float factor) const
    {
        return RayColor{ value / factor };
    }

    NFE_FORCE_INLINE RayColor& operator += (const RayColor& other)
    {
        value += other.value;
        return *this;
    }

    NFE_FORCE_INLINE RayColor& MulAndAccumulate(const RayColor& a, const RayColor& b)
    {
        value = Wavelength::ValueType::MulAndAdd(a.value, b.value, value);
        return *this;
    }

    NFE_FORCE_INLINE RayColor& MulAndAccumulate(const RayColor& a, const float b)
    {
        value = Wavelength::ValueType::MulAndAdd(a.value, b, value);
        return *this;
    }

    NFE_FORCE_INLINE RayColor& operator *= (const RayColor& other)
    {
        value *= other.value;
        return *this;
    }

    NFE_FORCE_INLINE RayColor& operator *= (const float factor)
    {
        value *= factor;
        return *this;
    }

    NFE_FORCE_INLINE bool AlmostZero() const
    {
        return Wavelength::ValueType::AlmostEqual(value, Wavelength::ValueType::Zero());
    }

    NFE_FORCE_INLINE float Max() const
    {
        Wavelength::ValueType maskedValue = value;
#ifndef NFE_ENABLE_SPECTRAL_RENDERING
        maskedValue &= Math::Vector4::MakeMask<1,1,1,0>();
#endif
        return maskedValue.HorizontalMax()[0];
    }

    NFE_FORCE_INLINE bool IsValid() const
    {
#ifndef NFE_ENABLE_SPECTRAL_RENDERING
        // exception: in spectral rendering these values can get below zero due to RGB->Spectrum conversion
        if (!(value >= Wavelength::ValueType::Zero()).All())
        {
            return false;
        }
#endif
        return value.IsValid();
    }

    NFE_FORCE_INLINE static const RayColor Lerp(const RayColor& a, const RayColor& b, const float factor)
    {
        return RayColor{ Wavelength::ValueType::Lerp(a.value, b.value, factor) };
    }

    // calculate ray color values for given wavelength and linear RGB values
    static const RayColor BlackBody(const Wavelength& wavelength, const float temperature);

    // calculate ray color values for given wavelength and spectrum
    static const RayColor Resolve(const Wavelength& wavelength, const Spectrum& spectrum);

    // convert to CIE XYZ / RGB tristimulus values
    // NOTE: when spectral rendering is disabled, this function does nothing (returns RGB values)
    const Math::Vector4 ConvertToTristimulus(const Wavelength& wavelength) const;
};

NFE_FORCE_INLINE const RayColor operator * (const float a, const RayColor& b)
{
    return b * a;
}


#ifndef NFE_ENABLE_SPECTRAL_RENDERING

NFE_FORCE_INLINE const Math::Vector4 RayColor::ConvertToTristimulus(const Wavelength&) const
{
    return value;
}

NFE_FORCE_INLINE const RayColor RayColor::Resolve(const Wavelength&, const Spectrum& spectrum)
{
    return RayColor{ spectrum.rgbValues };
}

#endif // NFE_ENABLE_SPECTRAL_RENDERING


} // namespace RT
} // namespace NFE

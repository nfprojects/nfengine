#include "PCH.h"
#include "BlackBodyColor.h"
#include "../../Common/Reflection/ReflectionClassDefine.hpp"
#include "../../Common/Math/Constants.hpp"
#include "../../Common/Math/Transcendental.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::BlackBodyColor)
{
    NFE_CLASS_PARENT(NFE::RT::IColor);
    NFE_CLASS_MEMBER(mTemperature).Min(0.0f).Max(10000.0f);
    NFE_CLASS_MEMBER(mIntensity).Min(0.0f).Max(1.0e+9f).LogScale(10.0f);
    NFE_CLASS_MEMBER(mNormalize);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

BlackBodyColor::BlackBodyColor()
    : mTemperature(5778.0f) // Sun temperature
    , mIntensity(1.0f)
    , mNormalize(true)
{}

bool BlackBodyColor::IsValid() const
{
    if (!Math::IsValid(mTemperature) || mTemperature < 0.0f)
    {
        return false;
    }
    if (!Math::IsValid(mIntensity) || mIntensity < 0.0f)
    {
        return false;
    }

    return true;
}

const RayColor BlackBodyColor::Resolve(const Wavelength& wavelength) const
{
    if (mTemperature < FLT_EPSILON)
    {
        return RayColor::Zero();
    }

    RayColor result;

    using namespace constants;
    constexpr float c1 = 2.0f * h * c * c;
    constexpr float c2 = h * c / k;

    {
        // wavelenght in meters
        const Wavelength::ValueType lambda =
            Wavelength::ValueType(Wavelength::Lower) +
            wavelength.value * (Wavelength::Higher - Wavelength::Lower);

        // Planck's law equation
        const Wavelength::ValueType term1 = Wavelength::ValueType(c1) / Pow<5>(lambda);
        const Wavelength::ValueType term2 = FastExp(Wavelength::ValueType(c2) / (lambda * mTemperature)) - Wavelength::ValueType(1.0f);
        result.value = term1 / term2;
    }

    // Wien's law
    if (mNormalize)
    {
        const float peakWavelength = 2.89777e-3f / mTemperature;

        // Planck's law equation
        const float term1 = c1 / Pow<5>(peakWavelength);
        const float term2 = expf(c2 / (peakWavelength * mTemperature)) - 1.0f;
        result *= 1.0f / (float)(term1 / term2);
    }
    else
    {
        // normalize for T = 5778K
        result.value *= 3.7912654e-14f;
    }

    return result * mIntensity;
}

} // namespace RT
} // namespace NFE

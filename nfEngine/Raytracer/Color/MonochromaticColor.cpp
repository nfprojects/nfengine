#include "PCH.h"
#include "MonochromaticColor.h"
#include "../../nfCommon/Reflection/ReflectionClassDefine.hpp"
#include "../../nfCommon/Math/Transcendental.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::MonochromaticColor)
{
    NFE_CLASS_PARENT(NFE::RT::IColor);
    NFE_CLASS_MEMBER(mWavelength).Min(300.0f).Max(800.0f);
    NFE_CLASS_MEMBER(mVariance).Min(1.0f).Max(1000.0f);
    NFE_CLASS_MEMBER(mIntensity).Min(0.0f).Max(1.0e+9f).LogScale(10.0f);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

MonochromaticColor::MonochromaticColor()
    : mWavelength(525.0f)
    , mVariance(10.0f)
    , mIntensity(1.0f)
{}

bool MonochromaticColor::IsValid() const
{
    if (!Math::IsValid(mWavelength) || mWavelength < 0.0f)
    {
        return false;
    }
    if (!Math::IsValid(mVariance) || mVariance < 0.0f)
    {
        return false;
    }
    if (!Math::IsValid(mIntensity) || mIntensity < 0.0f)
    {
        return false;
    }
    return true;
}

const RayColor MonochromaticColor::Resolve(const Wavelength& wavelength) const
{
    RayColor result;

    // wavelenght in meters
    const Wavelength::ValueType lambda =
        Wavelength::ValueType(Wavelength::Lower) +
        wavelength.value * (Wavelength::Higher - Wavelength::Lower);

    // Gaussian distribution
    const Wavelength::ValueType diff = 1.0e9f * lambda - Wavelength::ValueType(mWavelength);
    result.value = FastExp(-Sqr(diff) / mVariance);

    return result * mIntensity;
}

} // namespace RT
} // namespace NFE

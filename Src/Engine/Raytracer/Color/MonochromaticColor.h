#pragma once

#include "Color.h"

namespace NFE {
namespace RT {

// single-wavelength color
class NFE_RAYTRACER_API MonochromaticColor : public IColor
{
    NFE_DECLARE_POLYMORPHIC_CLASS(MonochromaticColor);

public:
    MonochromaticColor();

    virtual const RayColor Resolve(const Wavelength& wavelength) const override;
    virtual bool IsValid() const override;

private:
    float mWavelength;
    float mVariance;
    float mIntensity;
};

} // namespace RT
} // namespace NFE

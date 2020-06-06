#pragma once

#include "Color.h"

namespace NFE {
namespace RT {

class NFE_RAYTRACER_API BlackBodyColor : public IColor
{
    NFE_DECLARE_POLYMORPHIC_CLASS(BlackBodyColor)

public:

    BlackBodyColor();

    virtual const RayColor Resolve(const Wavelength& wavelength) const override;
    virtual bool IsValid() const override;

private:
    float mTemperature; // in Kelvins
    float mIntensity;
    bool mNormalize;
};

} // namespace RT
} // namespace NFE

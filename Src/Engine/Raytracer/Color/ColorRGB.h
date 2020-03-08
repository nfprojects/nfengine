#pragma once

#include "Color.h"
#include "../../Common/Math/LdrColor.hpp"
#include "../../Common/Math/HdrColor.hpp"

namespace NFE {
namespace RT {

// basic RGB color implementation
class NFE_RAYTRACER_API ColorRGB : public IColor
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ColorRGB);

public:
    NFE_FORCE_INLINE ColorRGB()
    {
        mColor = Math::HdrColorRGB(1.0f, 1.0f, 1.0f);
    }

    NFE_FORCE_INLINE ColorRGB(const Math::LdrColorRGB& color) : mColor(color) { }
    NFE_FORCE_INLINE ColorRGB(const Math::HdrColorRGB& color) : mColor(color) { }

    virtual const RayColor Resolve(const Wavelength& wavelength) const override;
    virtual bool IsValid() const override;

private:
    Math::HdrColorRGB mColor;

    // TODO different color primaries
};

} // namespace RT
} // namespace NFE

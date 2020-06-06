#pragma once

#include "Light.h"

namespace NFE {
namespace RT {

class SpotLight : public ILight
{
    NFE_DECLARE_POLYMORPHIC_CLASS(SpotLight)

public:
    NFE_RAYTRACER_API SpotLight(const Math::HdrColorRGB& color, const float angle);

    virtual const Math::Box GetBoundingBox() const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual Flags GetFlags() const override final;

private:
    float mAngle;
    float mCosAngle;
    bool mIsDelta; // true for 'laser' light
};

} // namespace RT
} // namespace NFE

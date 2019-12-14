#pragma once

#include "Light.h"

namespace NFE {
namespace RT {

class PointLight : public ILight
{
    NFE_DECLARE_POLYMORPHIC_CLASS(PointLight);

public:
    NFE_RAYTRACER_API PointLight(const Math::HdrColorRGB& color = Math::HdrColorRGB(1.0f));

    virtual const Math::Box GetBoundingBox() const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual Flags GetFlags() const override final;

private:

    // TODO texture
};

} // namespace RT
} // namespace NFE

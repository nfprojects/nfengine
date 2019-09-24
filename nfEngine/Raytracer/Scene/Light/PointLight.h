#pragma once

#include "Light.h"

namespace NFE {
namespace RT {

class PointLight : public ILight
{
public:
    NFE_RAYTRACER_API PointLight(const Math::Vector4& color);

    virtual Type GetType() const override;
    virtual const Math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const Math::Ray& ray, float& outDistance) const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual Flags GetFlags() const override final;

private:

    // TODO texture
};

} // namespace RT
} // namespace NFE

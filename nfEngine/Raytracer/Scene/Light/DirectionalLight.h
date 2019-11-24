#pragma once

#include "Light.h"

namespace NFE {
namespace RT {

class DirectionalLight : public ILight
{
    NFE_DECLARE_POLYMORPHIC_CLASS(DirectionalLight);

public:
    NFE_RAYTRACER_API DirectionalLight() = default;

    NFE_RAYTRACER_API DirectionalLight(const Math::HdrColorRGB& color, const float angle = 0.2f);

    virtual const Math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const Math::Ray& ray, float& outDistance) const override;
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const override;
    virtual const RayColor GetRadiance(const RadianceParam& param, float* outDirectPdfA, float* outEmissionPdfW) const override;
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const override;
    virtual Flags GetFlags() const override final;

    const Math::Vector4 SampleDirection(const Math::Float2 sample, float& outPdf) const;

private:
    float mCosAngle;
    bool mIsDelta;
};

} // namespace RT
} // namespace NFE

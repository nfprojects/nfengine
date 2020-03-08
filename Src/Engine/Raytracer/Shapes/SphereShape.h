#pragma once

#include "Shape.h"

namespace NFE {
namespace RT {

class SphereShape : public IShape
{
    NFE_DECLARE_POLYMORPHIC_CLASS(SphereShape);

public:
    NFE_RAYTRACER_API SphereShape(const float radius = 1.0f);

private:
    virtual const Math::Box GetBoundingBox() const override;
    virtual float GetSurfaceArea() const override;
    virtual bool Intersect(const Math::Ray& ray, RenderingContext& renderingCtx, ShapeIntersection& outResult) const override;
    virtual bool Intersect(const Math::Vector4& point) const override;
    virtual const Math::Vector4 Sample(const Math::Float3& u, Math::Vector4* outNormal, float* outPdf) const override;
    virtual bool Sample(const Math::Vector4& ref, const Math::Float3& u, ShapeSampleResult& result) const override;
    virtual float Pdf(const Math::Vector4& ref, const Math::Vector4& point) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    virtual bool OnPropertyChanged(const Common::StringView propertyName) override;

    double mRadiusD;
    float mRadius;
    float mInvRadius;
};

} // namespace RT
} // namespace NFE

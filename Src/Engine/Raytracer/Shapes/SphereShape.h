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
    virtual bool Intersect(const Math::Vec4f& point) const override;
    virtual const Math::Vec4f Sample(const Math::Vec3f& u, Math::Vec4f* outNormal, float* outPdf) const override;
    virtual bool Sample(const Math::Vec4f& ref, const Math::Vec3f& u, ShapeSampleResult& result) const override;
    virtual float Pdf(const Math::Vec4f& ref, const Math::Vec4f& point) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    virtual bool OnPropertyChanged(const Common::StringView propertyName) override;

    double mRadiusD;
    float mRadius;
    float mInvRadius;
};

} // namespace RT
} // namespace NFE

#pragma once

#include "Shape.h"

namespace NFE {
namespace RT {

class CylinderShape : public IShape
{
    NFE_DECLARE_POLYMORPHIC_CLASS(CylinderShape);

public:
    NFE_RAYTRACER_API CylinderShape(const float radius = 1.0f, const float height = 1.0f);

private:
    virtual const Math::Box GetBoundingBox() const override;
    virtual float GetSurfaceArea() const override;
    virtual bool Intersect(const Math::Ray& ray, RenderingContext& renderingCtx, ShapeIntersection& outResult) const override;
    virtual bool Intersect(const Math::Vec4f& point) const override;
    virtual const Math::Vec4f Sample(const Math::Vec3f& u, Math::Vec4f* outNormal, float* outPdf) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    virtual bool OnPropertyChanged(const Common::StringView propertyName) override;

    float mRadius;
    float mInvRadius;
    float mHeight;
};

} // namespace RT
} // namespace NFE

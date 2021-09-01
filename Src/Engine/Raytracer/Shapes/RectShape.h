#pragma once

#include "Shape.h"

namespace NFE {
namespace RT {

class RectShape : public IShape
{
    NFE_DECLARE_POLYMORPHIC_CLASS(RectShape)

public:
    NFE_RAYTRACER_API RectShape(const Math::Vec2f size = Math::Vec2f(1.0f), const Math::Vec2f texScale = Math::Vec2f(1.0f));

private:
    virtual const Math::Box GetBoundingBox() const override;
    virtual float GetSurfaceArea() const override;
    virtual bool Intersect(const Math::Ray& ray, RenderingContext& renderingCtx, ShapeIntersection& outResult) const override;
    virtual const Math::Vec4f SampleSurface(const Math::Vec3f& u, Math::Vec4f* outNormal, float* outPdf) const override;
    virtual bool SampleSurface(const Math::Vec4f& ref, const Math::Vec3f& u, ShapeSampleResult& result) const override;
    virtual float Pdf(const Math::Vec4f& ref, const Math::Vec4f& point) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    Math::Vec2f mSize;
    Math::Vec2f mTextureScale;

    bool mEnableSolidAngleSampling;
};

} // namespace RT
} // namespace NFE

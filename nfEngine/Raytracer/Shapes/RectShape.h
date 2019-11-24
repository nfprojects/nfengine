#pragma once

#include "Shape.h"

namespace NFE {
namespace RT {

class RectShape : public IShape
{
    NFE_DECLARE_POLYMORPHIC_CLASS(RectShape);

public:
    NFE_RAYTRACER_API RectShape(const Math::Float2 size = Math::Float2(FLT_MAX), const Math::Float2 texScale = Math::Float2(1.0f));

private:
    virtual const Math::Box GetBoundingBox() const override;
    virtual float GetSurfaceArea() const override;
    virtual bool Intersect(const Math::Ray& ray, ShapeIntersection& outResult) const override;
    virtual const Math::Vector4 Sample(const Math::Float3& u, Math::Vector4* outNormal, float* outPdf = nullptr) const override;
    virtual bool Sample(const Math::Vector4& ref, const Math::Float3& u, ShapeSampleResult& result) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    Math::Float2 mSize;
    Math::Float2 mTextureScale;

    bool mEnableSolidAngleSampling;
};

} // namespace RT
} // namespace NFE

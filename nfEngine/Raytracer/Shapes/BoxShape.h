#pragma once

#include "Shape.h"

namespace NFE {
namespace RT {

class BoxShape : public IShape
{
    NFE_DECLARE_POLYMORPHIC_CLASS(BoxShape);

public:
    NFE_RAYTRACER_API BoxShape(const Math::Vector4& size = Math::Vector4(0.5f, 0.5f, 0.5f));

private:
    virtual const Math::Box GetBoundingBox() const override;
    virtual float GetSurfaceArea() const override;
    virtual bool Intersect(const Math::Ray& ray, ShapeIntersection& outResult) const override;
    virtual bool Intersect(const Math::Vector4& point) const override;
    virtual const Math::Vector4 Sample(const Math::Float3& u, Math::Vector4* outNormal, float* outPdf = nullptr) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    virtual bool OnPropertyChanged(const Common::StringView propertyName) override;

    void OnSizeChanged();

    // half size
    Math::Vector4 mSize;
    Math::Vector4 mInvSize;

    // unnormalized face distribution (for box face sampling)
    Math::Float3 mFaceCdf;
};

} // namespace RT
} // namespace NFE

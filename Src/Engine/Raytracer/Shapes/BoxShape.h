#pragma once

#include "Shape.h"

namespace NFE {
namespace RT {

class BoxShape : public IShape
{
    NFE_DECLARE_POLYMORPHIC_CLASS(BoxShape)

public:
    NFE_RAYTRACER_API BoxShape(const Math::Vec4f& size = Math::Vec4f(0.5f, 0.5f, 0.5f));

private:
    virtual const Math::Box GetBoundingBox() const override;
    virtual float GetSurfaceArea() const override;
    virtual bool Intersect(const Math::Ray& ray, RenderingContext& renderingCtx, ShapeIntersection& outResult) const override;
    virtual bool Intersect(const Math::Vec4f& point) const override;
    virtual const Math::Vec4f SampleVolume(const Math::Vec3f& u) const override;
    virtual const Math::Vec4f SampleSurface(const Math::Vec3f& u, Math::Vec4f* outNormal, float* outPdf = nullptr) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    virtual bool OnPropertyChanged(const Common::StringView propertyName) override;

    void OnSizeChanged();

    // half size
    Math::Vec4f mSize;
    Math::Vec4f mInvSize;

    // unnormalized face distribution (for box face sampling)
    Math::Vec3f mFaceCdf;
};

} // namespace RT
} // namespace NFE

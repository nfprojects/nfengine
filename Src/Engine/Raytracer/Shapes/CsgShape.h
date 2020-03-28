#pragma once

#include "Shape.h"

#include "../../Common/Containers/SharedPtr.hpp"

namespace NFE {
namespace RT {

using ShapePtr = Common::SharedPtr<IShape>;

enum class CsgOperator : uint8
{
    Union,
    Difference,
    Intersection,
};

class CsgShape : public IShape
{
    NFE_DECLARE_POLYMORPHIC_CLASS(CsgShape);

public:
    NFE_RAYTRACER_API CsgShape();

    virtual const Math::Box GetBoundingBox() const override;

    virtual bool Intersect(const Math::Ray& ray, RenderingContext& renderingCtx, ShapeIntersection& outResult) const override;
    virtual const Math::Vec4f Sample(const Math::Vec3f& u, Math::Vec4f* outNormal, float* outPdf = nullptr) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

private:
    ShapePtr mShapeA;
    ShapePtr mShapeB;
    CsgOperator mOperator;
};

} // namespace RT
} // namespace NFE

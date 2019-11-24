#pragma once

#include "../Raytracer.h"
#include "../Utils/Memory.h"
#include "../Traversal/HitPoint.h"
#include "../../nfCommon/Math/Box.hpp"
#include "../../nfCommon/Math/Matrix4.hpp"
#include "../../nfCommon/Containers/SharedPtr.hpp"
#include "../../nfCommon/Memory/Aligned.hpp"
#include "../../nfCommon/Reflection/ReflectionClassDeclare.hpp"
#include "../../nfCommon/Reflection/Object.hpp"

#include <memory>

namespace NFE {
namespace RT {

struct ShapeIntersection
{
    float nearDist;
    float farDist;
    uint32 subObjectId = UINT32_MAX;
};

struct ShapeSampleResult
{
    Math::Vector4 position;
    Math::Vector4 normal;
    Math::Vector4 direction;
    float distance = -1.0f;
    float pdf = -1.0f;
    float cosAtSurface = -1.0f;
};

class IShape
    : public Common::Aligned<16>
    , public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(IShape);

public:
    NFE_RAYTRACER_API IShape();
    NFE_RAYTRACER_API virtual ~IShape();

    // get total surface area
    virtual float GetSurfaceArea() const;

    // traverse the object and find nearest intersection
    virtual void Traverse(const SingleTraversalContext& context, const uint32 objectID) const;

    // traverse the object and check if the ray is occluded
    virtual bool Traverse_Shadow(const SingleTraversalContext& context) const;

    // intersect with a ray and return hit points
    // TODO return array of all hit points along the ray
    virtual bool Intersect(const Math::Ray& ray, ShapeIntersection& outResult) const;

    // generate random point on the shape's surface
    // optionaly returns normal vector and sampling probability (with respect to area on the surface)
    virtual const Math::Vector4 Sample(const Math::Float3& u, Math::Vector4* outNormal = nullptr, float* outPdf = nullptr) const = 0;

    // generate random point on the shape's surface for given reference point
    // optionaly returns normal vector and sampling probability (with respect to solid angle visible from ref point)
    virtual bool Sample(const Math::Vector4& ref, const Math::Float3& u, ShapeSampleResult& result) const;
    virtual float Pdf(const Math::Vector4& ref, const Math::Vector4& point) const;

    // Calculate intersection data (tangent frame, tex coords, etc.) at given intersection point
    // NOTE: all calculations are performed in local space
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const = 0;

    // Get world-space bounding box
    virtual const Math::Box GetBoundingBox() const = 0;
};

} // namespace RT
} // namespace NFE

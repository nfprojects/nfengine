#pragma once

#include "../Raytracer.h"
#include "../Utils/Memory.h"
#include "../Traversal/HitPoint.h"
#include "../../Common/Math/Box.hpp"
#include "../../Common/Math/Matrix4.hpp"
#include "../../Common/Containers/SharedPtr.hpp"
#include "../../Common/Memory/Aligned.hpp"
#include "../../Common/Reflection/ReflectionClassDeclare.hpp"
#include "../../Common/Reflection/Object.hpp"

#include <memory>

namespace NFE {
namespace RT {

struct ShapeIntersection
{
    float nearDist;
    float farDist;
    float u;
    float v;
    uint32 subObjectId = UINT32_MAX;
};

struct ShapeSampleResult
{
    Math::Vec4f position;
    Math::Vec4f normal;
    Math::Vec4f direction;
    float distance = -1.0f;
    float pdf = -1.0f;
    float cosAtSurface = -1.0f;
};

class IShape
    : public Common::Aligned<16>
    , public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(IShape)

public:
    NFE_RAYTRACER_API IShape();
    NFE_RAYTRACER_API virtual ~IShape();

    // get total surface area
    virtual float GetSurfaceArea() const;

    // traverse the object and find nearest intersection
    virtual void Traverse(const SingleTraversalContext& context, const uint32 objectID) const;
    virtual void Traverse(const PacketTraversalContext& context, const uint32 objectID, const uint32 numActiveGroups) const;

    // traverse the object and check if the ray is occluded
    virtual bool Traverse_Shadow(const SingleTraversalContext& context) const;

    // intersect with a ray and return hit points
    // TODO return array of all hit points along the ray
    virtual bool Intersect(const Math::Ray& ray, RenderingContext& renderingCtx, ShapeIntersection& outResult) const;

    // check intersection with a point
    virtual bool Intersect(const Math::Vec4f& point) const;

    // must be called before using Sample() method
    virtual bool MakeSamplable();

    // generate random point on the shape's surface
    // optionaly returns normal vector and sampling probability (with respect to area on the surface)
    virtual const Math::Vec4f Sample(const Math::Vec3f& u, Math::Vec4f* outNormal = nullptr, float* outPdf = nullptr) const = 0;

    // generate random point on the shape's surface for given reference point
    // optionaly returns normal vector and sampling probability (with respect to solid angle visible from ref point)
    virtual bool Sample(const Math::Vec4f& ref, const Math::Vec3f& u, ShapeSampleResult& result) const;
    virtual float Pdf(const Math::Vec4f& ref, const Math::Vec4f& point) const;

    // Calculate intersection data (tangent frame, tex coords, etc.) at given intersection point
    // NOTE: all calculations are performed in local space
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const = 0;

    // Get world-space bounding box
    virtual const Math::Box GetBoundingBox() const = 0;
};

} // namespace RT
} // namespace NFE

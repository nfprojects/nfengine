#include "PCH.h"
#include "Shape.h"
#include "Traversal/TraversalContext.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::IShape)
{
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

IShape::IShape() = default;

IShape::~IShape() = default;

float IShape::GetSurfaceArea() const
{
    return 0.0f;
}

void IShape::Traverse(const SingleTraversalContext& context, const uint32 objectID) const
{
    ShapeIntersection intersection;

    if (Intersect(context.ray, context.context, intersection))
    {
        HitPoint& hitPoint = context.hitPoint;

        if (intersection.nearDist > 0.0f && intersection.nearDist < hitPoint.distance)
        {
            if (hitPoint.objectId != objectID || hitPoint.subObjectId != intersection.subObjectId)
            {
                hitPoint.Set(intersection.nearDist, objectID, intersection.subObjectId);
                return;
            }
        }

        if (intersection.farDist > 0.0f && intersection.farDist < hitPoint.distance)
        {
            if (hitPoint.objectId != objectID || hitPoint.subObjectId != intersection.subObjectId)
            {
                hitPoint.Set(intersection.farDist, objectID, intersection.subObjectId);
                return;
            }
        }
    }
}

void IShape::Traverse(const PacketTraversalContext&, const uint32, const uint32) const
{
    NFE_FATAL("Not implemented");
}

bool IShape::Traverse_Shadow(const SingleTraversalContext& context, const uint32 objectID) const
{
    ShapeIntersection intersection;

    if (!Intersect(context.ray, context.context, intersection))
    {
        return false;
    }

    return
        intersection.farDist > 0.0f && intersection.nearDist < context.hitPoint.distance
        && (context.hitPoint.objectId != objectID || context.hitPoint.subObjectId != intersection.subObjectId);
}

bool IShape::Intersect(const Ray&, RenderingContext&, ShapeIntersection&) const
{
    NFE_FATAL("This shape has no volume");
    return false;
}

bool IShape::Intersect(const Vec4f&) const
{
    NFE_FATAL("This shape has no volume");
    return false;
}

const Vec4f IShape::SampleVolume(const Vec3f&) const
{
    NFE_FATAL("This shape has no volume");
    return Vec4f::Zero();
}

bool IShape::SampleSurface(const Vec4f& ref, const Vec3f& u, ShapeSampleResult& result) const
{
    result.position = SampleSurface(u, &result.normal);

    NFE_ASSERT(Abs(1.0f - result.normal.SqrLength3()) < 0.001f, "Normal vector must be normalized");

    Vec4f dir = ref - result.position;
    const float sqrDistance = dir.SqrLength3();

    // compute PDF with respect to solid angle
    if (sqrDistance > Sqr(FLT_EPSILON))
    {
        const float distance = sqrtf(sqrDistance);
        dir /= distance;

        const float cosNormalDir = Vec4f::Dot3(result.normal, dir);
        if (cosNormalDir > FLT_EPSILON)
        {
            const float invArea = 1.0f / GetSurfaceArea();
            result.pdf = invArea * sqrDistance / cosNormalDir;
            result.distance = distance;
            result.cosAtSurface = cosNormalDir;
            result.direction = -dir;
            return true;
        }
    }

    return false;
}

float IShape::Pdf(const Vec4f& ref, const Vec4f& point) const
{
    NFE_UNUSED(ref);
    NFE_UNUSED(point);

    return 1.0f / GetSurfaceArea();
}

bool IShape::MakeSamplable()
{
    return true;
}

} // namespace RT
} // namespace NFE

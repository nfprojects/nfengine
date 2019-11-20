#include "PCH.h"
#include "Shape.h"
#include "Traversal/TraversalContext.h"
#include "../nfCommon/Reflection/ReflectionClassDefine.hpp"


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

    if (Intersect(context.ray, intersection))
    {
        HitPoint& hitPoint = context.hitPoint;

        if (intersection.nearDist > 0.0f && intersection.nearDist < hitPoint.distance)
        {
            //if (hitPoint.filterObjectId != objectID || hitPoint.filterSubObjectId != side)
            {
                hitPoint.Set(intersection.nearDist, objectID, intersection.subObjectId);
                return;
            }
        }

        if (intersection.farDist > 0.0f && intersection.farDist < hitPoint.distance)
        {
            //if (hitPoint.filterObjectId != objectID || hitPoint.filterSubObjectId != side)
            {
                hitPoint.Set(intersection.farDist, objectID, intersection.subObjectId);
                return;
            }
        }
    }
}

bool IShape::Traverse_Shadow(const SingleTraversalContext& context) const
{
    ShapeIntersection intersection;

    if (!Intersect(context.ray, intersection))
    {
        return false;
    }

    return intersection.farDist > 0.0f && intersection.nearDist < context.hitPoint.distance;
}

bool IShape::Intersect(const Ray&, ShapeIntersection&) const
{
    NFE_FATAL("This shape has no volume");
    return false;
}

bool IShape::Intersect(const Math::Vector4&) const
{
    NFE_FATAL("This shape has no volume");
    return false;
}

bool IShape::Sample(const Vector4& ref, const Float3& u, ShapeSampleResult& result) const
{
    result.position = Sample(u, &result.normal);

    Vector4 dir = ref - result.position;
    const float sqrDistance = dir.SqrLength3();

    // compute PDF with respect to solid angle
    if (sqrDistance > Sqr(FLT_EPSILON))
    {
        const float distance = sqrtf(sqrDistance);
        dir /= distance;

        const float cosNormalDir = Vector4::Dot3(result.normal, dir);
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

float IShape::Pdf(const Math::Vector4& ref, const Math::Vector4& point) const
{
    NFE_UNUSED(ref);
    NFE_UNUSED(point);

    return 1.0f / GetSurfaceArea();
}

} // namespace RT
} // namespace NFE

#include "PCH.hpp"
#include "Geometry.hpp"
#include "Sphere.hpp"
#include "Frustum.hpp"
#include "Transcendental.hpp"

namespace NFE {
namespace Math {

float ClosestPointOnSegment(const Vec4f& p, const Vec4f& p1, const Vec4f& p2, Vec4f& out)
{
    Vec4f ab = p2 - p1;
    float dot = Vec4f::Dot3(p - p1, ab);
    float len_sq = Vec4f::Dot3(ab, ab);
    float param = dot / len_sq;

    Vec4f result;
    if (param < 0.0f)
        result = p1;
    else if (param > 1.0f)
        result = p2;
    else
        result = p1 + param * ab;
    out = result;

    return (result - p).Length3();
}

// Box-box intersection test
template<> NFCOMMON_API
bool Intersect(const Box& box1, const Box& box2)
{
    return (((box1.min < box2.max) & (box2.min < box1.max)).GetMask() & 7) == 7;
}

template<> NFCOMMON_API
IntersectionResult IntersectEx(const Box& box1, const Box& box2)
{
    if ((box1.min > box2.max).GetMask() & 0x7)
    {
        return IntersectionResult::Outside;
    }

    if ((box2.min > box1.max).GetMask() & 0x7)
    {
        return IntersectionResult::Outside;
    }

    if ((((box1.min > box2.min) & (box2.max > box1.max)).GetMask() & 7) == 7)
    {
        return IntersectionResult::Inside;
    }

    return IntersectionResult::Intersect;
}

// Box-frustum intersection test
template<> NFCOMMON_API
bool Intersect(const Box& box, const Frustum& frustum)
{
    // test frustum vertices against box planes
    int maxMask = 0x7;
    int minMask = 0x7;
    for (int i = 0; i < 8; i++)
    {
        maxMask &= (frustum.verticies[i] > box.max).GetMask();
        minMask &= (frustum.verticies[i] < box.min).GetMask();
    }

    if (maxMask | minMask)
        return false;

    // test frustum planes against box vertices
    for (int i = 0; i < 6; i++)
    {
        const Plane& plane = frustum.planes[i];
        const Vec4f vmax = Vec4f::Select(box.max, box.min, plane.v < Vec4f::Zero());
        if (!plane.Side(vmax))
        {
            return false;
        }
    }

    return true;
}

template<> NFCOMMON_API
bool Intersect(const Frustum& frustum, const Box& box)
{
    return Intersect(box, frustum);
}

template<> NFCOMMON_API
IntersectionResult IntersectEx(const Box& box, const Frustum& frustum)
{
    int numPlanes = 0;

    // test frustum vertices against box planes
    int maxMask = 0x7;
    int minMask = 0x7;
    for (int i = 0; i < 8; i++)
    {
        maxMask &= (frustum.verticies[i] > box.max).GetMask();
        minMask &= (frustum.verticies[i] < box.min).GetMask();
    }

    if (maxMask | minMask)
        return IntersectionResult::Outside;

    // test frustum planes against box vertices
    for (int i = 0; i < 6; i++)
    {
        const Plane& plane = frustum.planes[i];
        const Vec4f vmax = Vec4f::Select(box.max, box.min, plane.v < Vec4f::Zero());
        const Vec4f vmin = Vec4f::Select(box.min, box.max, plane.v < Vec4f::Zero());

        if (!plane.Side(vmax))
            return IntersectionResult::Outside;

        if (!plane.Side(vmin))
            numPlanes++;
    }

    return (numPlanes == 6) ? IntersectionResult::Inside : IntersectionResult::Intersect;
}

// Frustum-frustum intersection test
template<> NFCOMMON_API
bool Intersect(const Frustum& f1, const Frustum& f2)
{
    if (!Intersect(f1.boundingBox, f2.boundingBox))
        return false;

    //f1 points vs f2 planes
    for (int i = 0; i < 6; i++)
    {
        int count = 0;

        for (int j = 0; j < 8; j++)
            if (!f2.planes[i].Side(f1.verticies[j]))
                count++;

        if (count == 8)
            return false;
    }

    //f2 points vs f1 planes
    for (int i = 0; i < 6; i++)
    {
        int count = 0;

        for (int j = 0; j < 8; j++)
            if (!f1.planes[i].Side(f2.verticies[j]))
                count++;

        if (count == 8)
            return false;
    }

    // TODO edge-edge case

    return true;
}

// Point-frustum intersection test
template<> NFCOMMON_API
bool Intersect(const Vec4f& point, const Frustum& frustum)
{
    for (int i = 0; i < 6; i++)
    {
        if (!frustum.planes[i].Side(point))
        {
            return false;
        }
    }

    return true;
}

// Triangle-frustum intersection test
template<> NFCOMMON_API
bool Intersect(const Triangle& tri, const Frustum& frustum)
{
    // frustum planes vs. triangle verticies
    for (int i = 0; i < 6; i++)
    {
        const bool outside0 = !frustum.planes[i].Side(tri.v0);
        const bool outside1 = !frustum.planes[i].Side(tri.v1);
        const bool outside2 = !frustum.planes[i].Side(tri.v2);
        if (outside0 && outside1 && outside2)
        {
            return false;
        }
    }

    // triangle plane vs. frustum vertices
    const Plane plane(tri.v0, tri.v1, tri.v2);
    int tmpSide = 0;
    for (int i = 0; i < 8; i++)
    {
        const bool side = plane.Side(frustum.verticies[i]);

        if (!side && tmpSide > 0)
            return true;

        if (side && tmpSide < 0)
            return true;

        if (tmpSide == 0)
            tmpSide = side ? 1 : -1;
    }

    return false;
}

// Point-sphere intersection test
template<> NFCOMMON_API
bool Intersect(const Vec4f& point, const Sphere& sphere)
{
    Vec4f segment = point - sphere.origin;
    return Vec4f::Dot3(segment, segment) <= sphere.r * sphere.r;
}

// Sphere-sphere intersection test
template<> NFCOMMON_API
bool Intersect(const Sphere& sphere1, const Sphere& sphere2)
{
    Vec4f segment = sphere1.origin - sphere2.origin;
    float radiiSum = sphere1.r + sphere2.r;
    return Vec4f::Dot3(segment, segment) <= radiiSum * radiiSum;
}

// Box-sphere intersection test
template<> NFCOMMON_API
bool Intersect(const Box& box, const Sphere& sphere)
{
    auto check = [&] (const float pn, const float bmin, const float bmax)
    {
        float out = 0.0f;
        float v = pn;

        if (v < bmin)
        {
            float val = (bmin - v);
            out += val * val;
        }

        if (v > bmax)
        {
            float val = (v - bmax);
            out += val * val;
        }

        return out;
    };

    // squared distance
    float sqrDist = check(sphere.origin[0], box.min[0], box.max[0]) +
        check(sphere.origin[1], box.min[1], box.max[1]) +
        check(sphere.origin[2], box.min[2], box.max[2]);

    return sqrDist <= sphere.r * sphere.r;
}

// Frustum-sphere intersection test
template<> NFCOMMON_API
bool Intersect(const Frustum& frustum, const Sphere& sphere)
{
    // test frustum planes against the sphere
    for (int i = 0; i < 6; i++)
    {
        Plane plane = frustum.planes[i];
        plane.v.w += sphere.r;
        if (!plane.Side(sphere.origin))
            return false;
    }

    // find the nearest point on frustum's edges to the sphere center
    // TODO: optimize - each ClosestPointOnSegment() call does division
    Vec4f nearest;
    float dist = std::numeric_limits<float>::max();

    auto check = [&] (const int i, const int j)
    {
        Vec4f tmpNearest;
        float tmpDist;
        tmpDist = ClosestPointOnSegment(sphere.origin, frustum.verticies[i], frustum.verticies[j],
            tmpNearest);
        if (tmpDist < dist)
        {
            dist = tmpDist;
            nearest = tmpNearest;
        }
    };

    /// "front"
    check(0, 1);
    check(1, 3);
    check(3, 2);
    check(2, 0);

    /// "back"
    check(4, 5);
    check(5, 7);
    check(7, 6);
    check(6, 4);

    /// "rest"
    check(0, 4);
    check(1, 5);
    check(2, 6);
    check(3, 7);

    // sphere colliding with edge
    if (dist < sphere.r)
        return true;

    // test sphere against frustum vertices
    Plane plane((nearest - sphere.origin).Normalized3(), sphere.origin);
    plane.v.w -= sphere.r;

    for (int i = 0; i < 8; ++i)
    {
        if (!plane.Side(frustum.verticies[i]))
            return true;
    }

    return false;
}

const Vec4f CartesianToSphericalCoordinates(const Vec4f& input)
{
    const float theta = FastACos(Clamp(input.y, -1.0f, 1.0f));
    const float phi = Abs(input.x) > FLT_EPSILON ? FastATan2(input.z, input.x) : 0.0f;
    return Vec4f(phi / (2.0f * NFE_MATH_PI) + 0.5f, theta / NFE_MATH_PI, 0.0f, 0.0f);
}

void BuildOrthonormalBasis(const Vec4f& n, Vec4f& u, Vec4f& v)
{
    // algorithm based on "Building an Orthonormal Basis, Revisited" (2017) paper
    // by T. Duff, J. Burgess, P. Christensen, C. Hery, A. Kensler, M. Liani, and R. Villemin

    const float sign = CopySign(1.0f, n.z);
    const float a = -1.0f / (sign + n.z);

    u = Vec4f(
        1.0f + sign * n.x * n.x * a,
        sign * n.x * n.y * a,
        -sign * n.x);

    v = Vec4f(
        n.x * n.y * a,
        sign + n.y * n.y * a,
        -n.y);
}

} // namespace Math
} // namespace NFE

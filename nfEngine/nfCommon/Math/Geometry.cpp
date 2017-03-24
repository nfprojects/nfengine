/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Geometry function definitions.
 */

#include "PCH.hpp"
#include "Geometry.hpp"
#include "Box.hpp"
#include "Sphere.hpp"
#include "Frustum.hpp"
#include "Triangle.hpp"

namespace NFE {
namespace Math {

float ClosestPointOnSegment(const Vector& p, const Vector& p1, const Vector& p2, Vector& out)
{
    Vector ab = p2 - p1;
    float dot = Vector::Dot3(p - p1, ab);
    float len_sq = Vector::Dot3(ab, ab);
    float param = dot / len_sq;

    Vector result;
    if (param < 0.0f)
        result = p1;
    else if (param > 1.0f)
        result = p2;
    else
        result = p1 + param * ab;
    out = result;

    return (result - p).Length3();
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
        maxMask &= Vector::GreaterMask(frustum.verticies[i], box.max);
        minMask &= Vector::LessMask(frustum.verticies[i], box.min);
    }

    if (maxMask | minMask)
        return false;

    // test frustum planes against box vertices
    for (int i = 0; i < 6; i++)
    {
        Vector plane = frustum.planes[i];
        Vector vmax = Vector::SelectBySign(box.max, box.min, plane);
        if (!Vector::PlanePointSide(plane, vmax))
            return false;
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
        maxMask &= Vector::GreaterMask(frustum.verticies[i], box.max);
        minMask &= Vector::LessMask(frustum.verticies[i], box.min);
    }

    if (maxMask | minMask)
        return IntersectionResult::Outside;

    // test frustum planes against box vertices
    for (int i = 0; i < 6; i++)
    {
        Vector plane = frustum.planes[i];
        Vector vmax = Vector::SelectBySign(box.max, box.min, plane);
        Vector vmin = Vector::SelectBySign(box.min, box.max, plane);

        if (!Vector::PlanePointSide(plane, vmax))
            return IntersectionResult::Outside;

        if (!Vector::PlanePointSide(plane, vmin))
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
            if (!Vector::PlanePointSide(f2.planes[i], f1.verticies[j]))
                count++;

        if (count == 8)
            return false;
    }

    //f2 points vs f1 planes
    for (int i = 0; i < 6; i++)
    {
        int count = 0;

        for (int j = 0; j < 8; j++)
            if (!Vector::PlanePointSide(f1.planes[i], f2.verticies[j]))
                count++;

        if (count == 8)
            return false;
    }

    return true;
}

// Point-frustum intersection test
template<> NFCOMMON_API
bool Intersect(const Vector& point, const Frustum& frustum)
{
    for (int i = 0; i < 6; i++)
        if (!Vector::PlanePointSide(frustum.planes[i], point))
            return false;

    return true;
}

// Triangle-frustum intersection test
template<> NFCOMMON_API
bool Intersect(const Triangle& tri, const Frustum& frustum)
{
    // frustum planes vs. triangle verticies
    for (int i = 0; i < 6; i++)
    {
        bool outside = !Vector::PlanePointSide(frustum.planes[i], tri.v0);
        outside &= !Vector::PlanePointSide(frustum.planes[i], tri.v1);
        outside &= !Vector::PlanePointSide(frustum.planes[i], tri.v2);
        if (outside)
            return false;
    }

    // triangle plane vs. frustum vertices
    Vector plane = Vector::PlaneFromPoints(tri.v0, tri.v1, tri.v2);
    int tmpSide = 0;
    for (int i = 0; i < 8; i++)
    {
        bool side = Vector::PlanePointSide(plane, frustum.verticies[i]);

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
bool Intersect(const Vector& point, const Sphere& sphere)
{
    Vector segment = point - sphere.origin;
    return Vector::Dot3(segment, segment) <= sphere.r * sphere.r;
}

// Sphere-sphere intersection test
template<> NFCOMMON_API
bool Intersect(const Sphere& sphere1, const Sphere& sphere2)
{
    Vector segment = sphere1.origin - sphere2.origin;
    float radiiSum = sphere1.r + sphere2.r;
    return Vector::Dot3(segment, segment) <= radiiSum * radiiSum;
}

// Box-sphere intersection test
template<> NFCOMMON_API
bool Intersect(const Box& box, const Sphere& sphere)
{
    auto check = [&](const float pn, const float bmin, const float bmax)
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
        Vector plane = frustum.planes[i];
        plane.f[3] += sphere.r;
        if (!Vector::PlanePointSide(plane, sphere.origin))
            return false;
    }

    // find the nearest point on frustum's edges to the sphere center
    // TODO: optimize - each ClosestPointOnSegment() call does division
    Vector nearest;
    float dist = 1.0e+10f;

    auto check = [&](const int i, const int j)
    {
        Vector tmpNearest;
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
    Vector plane = Vector::PlaneFromNormalAndPoint((nearest - sphere.origin).Normalized3(), sphere.origin);
    plane.f[3] -= sphere.r;

    for (int i = 0; i < 8; ++i)
    {
        if (!Vector::PlanePointSide(plane, frustum.verticies[i]))
            return true;
    }

    return false;
}


} // namespace Math
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Geometry function definitions.
 */

#include "../PCH.hpp"
#include "Geometry.hpp"
#include "Box.hpp"
#include "Sphere.hpp"
#include "Frustum.hpp"
#include "Triangle.hpp"

// TODO: remove after porting Geometry module to non-SSE version
#include <smmintrin.h>

namespace NFE {
namespace Math {

float ClosestPointOnSegment(const Vector& p, const Vector& p1, const Vector& p2, Vector& out)
{
    Vector ab = p2 - p1;
    float dot = VectorDot3(p - p1, ab)[0];
    float len_sq = VectorDot3(ab, ab)[0];
    float param = dot / len_sq;

    Vector result;
    if (param < 0.0f)
        result = p1;
    else if (param > 1.0f)
        result = p2;
    else
        result = p1 + param * ab;
    out = result;

    return VectorLength3(result - p)[0];
}

// Box-box intersection test
template<> NFCOMMON_API
bool Intersect(const Box& box1, const Box& box2)
{
    __m128 vTemp = _mm_cmpge_ps(box1.min, box2.max);
    if (_mm_movemask_ps(vTemp) & 7)
        return false;

    vTemp = _mm_cmpge_ps(box2.min, box1.max);
    if (_mm_movemask_ps(vTemp) & 7)
        return false;

    return true;
}

// Box-point intersection test
template<> NFCOMMON_API
bool Intersect(const Box& box, const Vector& point)
{
    Vector cmpMax = _mm_cmpge_ps(box.max, point);
    Vector cmpMin = _mm_cmpge_ps(point, box.min);
    return (_mm_movemask_ps(_mm_and_ps(cmpMax, cmpMin)) & 7) == 7;
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
        maxMask &= _mm_movemask_ps(_mm_cmpgt_ps(frustum.verticies[i], box.max));
        minMask &= _mm_movemask_ps(_mm_cmplt_ps(frustum.verticies[i], box.min));
    }

    if (maxMask | minMask)
        return false;

    // test frustum planes against box vertices
    for (int i = 0; i < 6; i++)
    {
        // SSE4 version
        __m128 plane = frustum.planes[i];
        __m128 vmax = _mm_blendv_ps(box.max, box.min, plane);
        __m128 dot = _mm_dp_ps(vmax, plane, 0x71);
        dot = _mm_add_ss(dot, _mm_shuffle_ps(plane, plane, _MM_SHUFFLE(3, 3, 3, 3)));
        if (_mm_comilt_ss(dot, _mm_setzero_ps()))
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
        maxMask &= _mm_movemask_ps(_mm_cmpgt_ps(frustum.verticies[i], box.max));
        minMask &= _mm_movemask_ps(_mm_cmplt_ps(frustum.verticies[i], box.min));
    }

    if (maxMask | minMask)
        return IntersectionResult::Outside;

    // test frustum planes against box vertices
    for (int i = 0; i < 6; i++)
    {
        __m128 plane = frustum.planes[i];
        __m128 vmax = _mm_blendv_ps(box.max, box.min, plane);
        __m128 vmin = _mm_blendv_ps(box.min, box.max, plane);

        __m128 dot = _mm_dp_ps(vmax, plane, 0x71);
        dot = _mm_add_ss(dot, _mm_shuffle_ps(plane, plane, _MM_SHUFFLE(3, 3, 3, 3)));
        if (_mm_comilt_ss(dot, _mm_setzero_ps()))
            return IntersectionResult::Outside;

        dot = _mm_dp_ps(vmin, plane, 0x71);
        dot = _mm_add_ss(dot, _mm_shuffle_ps(plane, plane, _MM_SHUFFLE(3, 3, 3, 3)));
        numPlanes += _mm_comilt_ss(dot, _mm_setzero_ps());
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
            if (!PlanePointSide(f2.planes[i], f1.verticies[j]))
                count++;

        if (count == 8)
            return false;
    }

    //f2 points vs f1 planes
    for (int i = 0; i < 6; i++)
    {
        int count = 0;

        for (int j = 0; j < 8; j++)
            if (!PlanePointSide(f1.planes[i], f2.verticies[j]))
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
        if (!PlanePointSide(frustum.planes[i], point))
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
        bool outside = !PlanePointSide(frustum.planes[i], tri.v0);
        outside &= !PlanePointSide(frustum.planes[i], tri.v1);
        outside &= !PlanePointSide(frustum.planes[i], tri.v2);
        if (outside)
            return false;
    }

    // traingle plane vs. frustum vertices
    Vector plane = PlaneFromPoints(tri.v0, tri.v1, tri.v2);
    int tmpSide = 0;
    for (int i = 0; i < 8; i++)
    {
        bool side = PlanePointSide(plane, frustum.verticies[i]);

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
    __m128 segment = point - sphere.origin;
    __m128 radiiSum = _mm_set_ps1(sphere.r);
    return _mm_comile_ss(VectorDot3(segment, segment), _mm_mul_ss(radiiSum, radiiSum)) != 0;
}

// Sphere-sphere intersection test
template<> NFCOMMON_API
bool Intersect(const Sphere& sphere1, const Sphere& sphere2)
{
    __m128 segment = sphere1.origin - sphere2.origin;
    __m128 radiiSum = _mm_set_ps1(sphere1.r + sphere2.r);
    return _mm_comile_ss(VectorDot3(segment, segment), _mm_mul_ss(radiiSum, radiiSum)) != 0;
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
        if (!PlanePointSide(plane, sphere.origin))
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

    // sphere coliding with edge
    if (dist < sphere.r)
        return true;

    // test sphere against frustum vertices
    Vector plane = PlaneFromNormalAndPoint(VectorNormalize3(nearest - sphere.origin), sphere.origin);
    plane.f[3] -= sphere.r;

    for (int i = 0; i < 8; ++i)
    {
        if (!PlanePointSide(plane, frustum.verticies[i]))
            return true;
    }

    return false;
}


} // namespace Math
} // namespace NFE

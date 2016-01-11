/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Ray intersection functions.
 */

#include "../PCH.hpp"
#include "Geometry.hpp"
#include "Box.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Ray.hpp"

namespace NFE {
namespace Math {

// Ray-Box intersection functions =================================================================

NFE_INLINE bool RayBoxIntersectInline(const Ray& ray, const Box& box, Vector& dist)
{
    // TODO: non-SSE version

    // The algorithm is based on "slabs" method. More info can be found here:
    // http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm

    Vector lmin, lmax, tmp1, tmp2;

    // calculate all box planes distances
    tmp1 = (box.min - ray.origin) * ray.invDir;
    tmp2 = (box.max - ray.origin) * ray.invDir;
    lmin = VectorMin(tmp1, tmp2);
    lmax = VectorMax(tmp1, tmp2);

    // transpose (we need to calculate min and max of X, Y and Z)
    Vector lx = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(0, 0, 0, 0));
    Vector ly = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(1, 1, 1, 1));
    Vector lz = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(2, 2, 2, 2));

    // calculate minimum and maximum plane distances by taking min and max
    // of all 3 components
    lmin = _mm_max_ps(lx, _mm_max_ps(ly, lz));
    lmax = _mm_min_ps(lx, _mm_min_ps(ly, lz));
    dist = lmin.SplatX();

    // setup data for final comparison
    lmax = lmax.SplatZ();
    // lmin = [lmin, lmin, 0, 0]
    lmin = _mm_shuffle_ps(lmin, _mm_setzero_ps(), _MM_SHUFFLE(0, 0, 0, 0));

    // lmax >= lmin && lmax >= 0.0
    // (bits 0, 1)     (bit 2, 3)
    // The check below is a little bit redundant (we perform 4 comparisons), so
    // all 4 comparisons must return success (that's why we check if mask is 0xF).
    return _mm_movemask_ps(_mm_cmpge_ps(lmax, lmin)) == 0xF;
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Box& box)
{
    Vector unusedDist;
    return RayBoxIntersectInline(ray, box, unusedDist);
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Box& box, Vector& dist)
{
    return RayBoxIntersectInline(ray, box, dist);
}


// Ray-Triangle intersection functions ============================================================

NFE_INLINE bool RayTriangleIntersectInline(const Ray& ray, const Triangle& tri, Vector& dist)
{
    // TODO: non-SSE version

    // Based on "Fast, Minimum Storage Ray/Triangle Intersection" by Tomas M�ller and Ben Trumbore.

    // find vectors for two edges sharing v0
    Vector edge0 = tri.v1 - tri.v0;
    Vector edge1 = tri.v2 - tri.v0;
    // begin calculating determinant - also used to calculate U parameter
    Vector pvec = VectorCross3(ray.dir, edge1);
    // if determinant is near zero, ray lies in plane of triangle
    Vector det = VectorDot3(edge0, pvec);
    // calculate distance from vert0 to ray origin
    Vector tvec = ray.origin - tri.v0;
    // prepare to test V parameter
    Vector qvec = VectorCross3(tvec, edge0);
    // calculate U parameter
    Vector u = VectorDot3(tvec, pvec);
    // calculate V parameter
    Vector v = VectorDot3(ray.dir, qvec);
    // calculate t (distance)
    Vector t = VectorDot3(edge1, qvec);

    // prepare data to the final comparison
    Vector tmp1 = _mm_shuffle_ps(v, u, _MM_SHUFFLE(0, 0, 0, 0));
    Vector tmp2 = _mm_shuffle_ps(u + v, t, _MM_SHUFFLE(0, 0, 0, 0));
    tmp1 = _mm_shuffle_ps(tmp2, tmp1, _MM_SHUFFLE(2, 0, 2, 0));
    tmp1 = tmp1 / det;
    tmp2 = _mm_set_ss(1.0f);
    dist = tmp1.SplatY();

    // At this point, in tmp1 we have: [u, v, t, u + v]
    // and in tmp2 we have:            [0, 0, 0, 1].
    // The intersection occurs if (u > 0 && v > 0 && t > 0 && u + v < 1),
    // so when performing SSE comparison 3 upper components must return true,
    // and last false, which yelds to 0xE bit mask.
    return (_mm_movemask_ps(_mm_cmpgt_ps(tmp1, tmp2)) == 0xE);
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Triangle& tri)
{
    Vector unusedDist;
    return RayTriangleIntersectInline(ray, tri, unusedDist);
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Triangle& tri, Vector& dist)
{
    return RayTriangleIntersectInline(ray, tri, dist);
}


// Ray-Sphere intersection functions ==============================================================

NFE_INLINE bool RaySphereIntersectInline(const Ray& ray, const Sphere& sphere, Vector& dist)
{
    Vector d = sphere.origin - ray.origin;
    float v = VectorDot3f(ray.dir, d);
    float det = sphere.r * sphere.r - VectorDot3f(d, d) + v * v;

    if (det > 0.0f)
    {
        dist = VectorSplat(v - sqrtf(det));
        return true;
    }
    return false;
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Sphere& sphere)
{
    Vector unusedDist;

    return RaySphereIntersectInline(ray, sphere, unusedDist);
}

template<> NFCOMMON_API
bool Intersect(const Ray& ray, const Sphere& sphere, Vector& dist)
{
    return RaySphereIntersectInline(ray, sphere, dist);
}

} // namespace Math
} // namespace NFE

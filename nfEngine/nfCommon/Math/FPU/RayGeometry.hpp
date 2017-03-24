/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  FPU-specific ray intersection functions.
 */

#include "../Geometry.hpp"
#include "../Box.hpp"
#include "../Triangle.hpp"
#include "../Ray.hpp"

#if defined(NFE_USE_SSE)
#error "The header cannot be used when NFE_USE_SSE is defined!"
#endif

namespace NFE {
namespace Math {

NFE_INLINE bool RayBoxIntersectInline(const Ray& ray, const Box& box, Vector& dist)
{
    // The algorithm is based on "slabs" method. More info can be found here:
    // http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm

    Vector lmin, lmax, tmp1, tmp2;

    // calculate all box planes distances
    tmp1 = (box.min - ray.origin) * ray.invDir;
    tmp2 = (box.max - ray.origin) * ray.invDir;
    lmin = Vector::Min(tmp1, tmp2);
    lmax = Vector::Max(tmp1, tmp2);

    // calculate minimum and maximum plane distances by taking min and max
    // of all 3 components
    float minDist = Max(lmin[0], Max(lmin[1], lmin[2]));
    float maxDist = Min(lmax[0], Min(lmax[1], lmax[2]));
    dist = Vector::Splat(minDist);

    return maxDist >= minDist && maxDist >= 0.0f;
}

NFE_INLINE bool RayTriangleIntersectInline(const Ray& ray, const Triangle& tri, Vector& dist)
{
    // Based on "Fast, Minimum Storage Ray/Triangle Intersection" by Tomas Möller and Ben Trumbore.

    // find vectors for two edges sharing v0
    Vector edge0 = tri.v1 - tri.v0;
    Vector edge1 = tri.v2 - tri.v0;
    // begin calculating determinant - also used to calculate U parameter
    Vector pvec = Vector::Cross3(ray.dir, edge1);
    // calculate distance from vert0 to ray origin
    Vector tvec = ray.origin - tri.v0;
    // prepare to test V parameter
    Vector qvec = Vector::Cross3(tvec, edge0);
    // if determinant is near zero, ray lies in plane of triangle
    float det = Vector::Dot3(edge0, pvec);
    float u = Vector::Dot3(tvec, pvec);
    float v = Vector::Dot3(ray.dir, qvec);
    float t = Vector::Dot3(edge1, qvec);
    u /= det;
    v /= det;
    t /= det;
    dist = Vector::Splat(t);
    return u >= 0.0f && v >= 0.0f && t >= 0.0f && u + v <= 1.0f;
}

} // namespace Math
} // namespace NFE

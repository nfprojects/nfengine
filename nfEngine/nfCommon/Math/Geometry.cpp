/**
 * @file   Geometry.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Geometry function definitions.
 */

#include "../stdafx.hpp"
#include "Math.hpp"
#include <smmintrin.h>

namespace NFE {
namespace Math {

int IntersectBoxBox(const Box& box1, const Box& box2)
{
    __m128 vTemp = _mm_cmpge_ps(box1.min, box2.max);
    if (_mm_movemask_ps(vTemp) & 7)
        return 0;

    vTemp = _mm_cmpge_ps(box2.min, box1.max);
    if (_mm_movemask_ps(vTemp) & 7)
        return 0;

    return 1;
}

int IntersectBoxPoint(const Box& box, const Vector& point)
{
    Vector cmpMax = _mm_cmpge_ps(box.max, point);
    Vector cmpMin = _mm_cmpge_ps(point, box.min);
    return (_mm_movemask_ps(_mm_and_ps(cmpMax, cmpMin)) & 7) == 7;
}

int IntersectBoxesPoint(const Box* pBoxes, const Vector& point)
{
    Vector cmpMax = _mm_cmpge_ps(pBoxes[0].max, point);
    Vector cmpMin = _mm_cmple_ps(pBoxes[0].min, point);
    int resultA = (_mm_movemask_ps(_mm_and_ps(cmpMax, cmpMin)) & 7 ) == 7;

    cmpMax = _mm_cmpge_ps(pBoxes[1].max, point);
    cmpMin = _mm_cmple_ps(pBoxes[1].min, point);
    int resultB = (_mm_movemask_ps(_mm_and_ps(cmpMax, cmpMin)) & 7 ) == 7;

    return resultA | (resultB << 1);
}

int IntersectBoxFrustum(const Box& box, const Frustum& frustum)
{
    //frustum planes vs box verticies
    /*
    //my version
    for (int i = 0; i < 6; i++)
    {
        int count = 0;

        for (int j = 0; j<8; j++)
        {
            Vector vert = box.GetVertex(j);
            vert.f[3] = 1.0f;

            if (VectorDot4(frustum.planes[i], vert).f[0] < 0.0f)
                count++;
        }

        if (count == 8)
            return 0;
    }

    return 1;
    */

    // fast version
    Vector plane, vmax, dot;

    for (int i = 0; i < 6; i++)
    {
        // SSE4 version
        plane = frustum.planes[i];
        vmax = _mm_blendv_ps(box.max, box.min, plane);

        dot = _mm_dp_ps(vmax, plane, 0x71);
        dot = _mm_add_ss(dot, _mm_shuffle_ps(plane, plane, _MM_SHUFFLE(3, 3, 3, 3)));
        if (_mm_comilt_ss(dot, _mm_setzero_ps())) return 0;
    }

    return 1;
}

int IntersectFrustumFrustum(const Frustum& f1, const Frustum& f2)
{
    if (!IntersectBoxBox(f1.boundingBox, f2.boundingBox))
        return 0;

    //f1 points vs f2 planes
    for (int i = 0; i < 6; i++)
    {
        int count = 0;

        for (int j = 0; j < 8; j++)
        {
            Vector vert = f1.verticies[j];
            vert.f[3] = 1.0f;

            if (VectorDot4(f2.planes[i], vert).f[0] < 0.0f)
                count++;

        }

        if (count == 8) return 0;
    }

    //f2 points vs f1 planes
    for (int i = 0; i < 6; i++)
    {
        int count = 0;

        for (int j = 0; j < 8; j++)
        {
            Vector vert = f2.verticies[j];
            vert.f[3] = 1.0f;

            if (VectorDot4(f1.planes[i], vert).f[0] < 0.0f)
                count++;

        }

        if (count == 8) return 0;
    }

    return 1;
}


int IntersectTriangleFrustum(const Triangle& tri, const Frustum& frustum)
{
    int outside;

    //frustum planes vs triangle verticies
    for (int i = 0; i < 6; i++)
    {
        outside = VectorDot3(frustum.planes[i], tri.v0).f[0] + frustum.planes[i].f[3] > 0.0f;
        outside &= VectorDot3(frustum.planes[i], tri.v1).f[0] + frustum.planes[i].f[3] > 0.0f;
        outside &= VectorDot3(frustum.planes[i], tri.v2).f[0] + frustum.planes[i].f[3] > 0.0f;

        if (outside)
            return 0;
        //if (VectorDot4(planes[i].normal, vmax).m128_f32[0] >= 0.0f)
        //  ret = INTERSECT;
    }

    return 1;
}


int IntersectRayBox(const Box& box, const Ray& ray)
{
    __m128 lmin, lmax, tmp1, tmp2;
    tmp1 = _mm_sub_ps(box.min, ray.origin);
    tmp2 = _mm_sub_ps(box.max, ray.origin);
    tmp1 = _mm_mul_ps(tmp1, ray.invDir);
    tmp2 = _mm_mul_ps(tmp2, ray.invDir);
    lmin = _mm_min_ps(tmp1, tmp2);
    lmax = _mm_max_ps(tmp1, tmp2);

    __m128 lx = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(0, 0, 0, 0));
    __m128 ly = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(1, 1, 1, 1));
    __m128 lz = _mm_shuffle_ps(lmin, lmax, _MM_SHUFFLE(2, 2, 2, 2));

    lmin = _mm_max_ps(lx, _mm_max_ps(ly, lz));
    lmax = _mm_min_ps(lx, _mm_min_ps(ly, lz));
    lmax = _mm_shuffle_ps(lmax, lmax, _MM_SHUFFLE(2, 2, 2, 2));
    lmin = _mm_shuffle_ps(lmin, _mm_setzero_ps(), _MM_SHUFFLE(0, 0, 0, 0));

    return _mm_movemask_ps(_mm_cmpge_ps(lmax, lmin)) == 15;
}

//ultra fast ray-box intersection test
int IntersectRayBox(const Box& box, const Ray& ray, RayBoxSegment* pSegment)
{
    __m128 tmp1 = _mm_mul_ps(_mm_sub_ps(box.min, ray.origin), ray.invDir);
    __m128 tmp2 = _mm_mul_ps(_mm_sub_ps(box.max, ray.origin), ray.invDir);
    __m128 lmin = _mm_min_ps(tmp1, tmp2);
    __m128 lmax = _mm_max_ps(tmp1, tmp2);

    __m128 lx = _mm_shuffle_ps(lmin, lmax, 0x0);
    __m128 ly = _mm_shuffle_ps(lmin, lmax, 0x55);
    __m128 lz = _mm_shuffle_ps(lmin, lmax, 0xAA);

    lmin = _mm_max_ps(lx, _mm_max_ps(ly, lz));
    lmax = _mm_min_ps(lx, _mm_min_ps(ly, lz));
    lmax = _mm_shuffle_ps(lmax, lmax, 0xAA);
    pSegment->nearDist = _mm_shuffle_ps(lmin, lmin, 0);
    pSegment->farDist = lmax;


    lmin = _mm_shuffle_ps(lmin, _mm_setzero_ps(), _MM_SHUFFLE(0, 0, 0, 0));
    return _mm_movemask_ps(_mm_cmpge_ps(lmax, lmin)) == 15;
}

int IntersectRayTriangle(const Ray& ray, const Triangle& tri, Vector* pDist)
{
    Vector edge0 = tri.v1 - tri.v0;
    Vector edge1 = tri.v2 - tri.v0;

    Vector pvec = VectorCross3(ray.dir, edge1);

    Vector det = VectorDot3(edge0, pvec);

    Vector tvec = ray.origin - tri.v0;
    Vector qvec = VectorCross3(tvec, edge0);

    Vector u = VectorDot3(tvec, pvec);
    Vector v = VectorDot3(ray.dir, qvec);
    Vector t = VectorDot3(edge1, qvec);
    Vector uv_sum = _mm_add_ps(u, v);

    Vector tmp1 = _mm_shuffle_ps(v, u, _MM_SHUFFLE(0, 0, 0, 0));
    Vector tmp2 = _mm_shuffle_ps(uv_sum, t, _MM_SHUFFLE(0, 0, 0, 0));
    tmp1 = _mm_shuffle_ps(tmp2, tmp1, _MM_SHUFFLE(2, 0, 2, 0));
    tmp1 = _mm_div_ps(tmp1, det);

    tmp2 = _mm_set_ss(1.0f);

    (*pDist) = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 1, 1, 1));
    return (_mm_movemask_ps(_mm_cmpgt_ps(tmp1, tmp2)) == 14);
}

} // namespace Math
} // namespace NFE

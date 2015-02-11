/**
 * @file   Geometry.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Geometry objects declarations.
 */

#pragma once

namespace NFE {
namespace Math {

//Axis Aligned Box
struct NFE_ALIGN(16) Box
{
    Vector min;
    Vector max;

    NFE_INLINE Box() : min(), max() {};
    NFE_INLINE Box(const Vector& min_, const Vector& max_) : min(min_), max(max_) {};

    // mege boxes
    NFE_INLINE Box(const Box& a, const Box& b)
    {
        min = VectorMin(a.min, b.min);
        max = VectorMax(a.max, b.max);
    }

    NFE_INLINE Vector GetCenter() const;
    NFE_INLINE Vector GetVertex(int id) const;
    NFE_INLINE Vector SupportVertex(const Vector& dir) const;
    NFE_INLINE void MakeFromPoints(const Vector* pPoints, int number);
    NFE_INLINE float SurfaceArea() const;
    NFE_INLINE float Volume() const;
};

//Frustum
struct NFE_ALIGN(16) Frustum
{
    Vector planes[6];
    Vector verticies[8];
    Box boundingBox;

    NFE_INLINE void CalculatePlanes();
    NFE_INLINE float SupportVertex(const Vector& dir);
};

struct NFE_ALIGN(16) Sphere
{
    Vector origin;
    float r;

    NFE_INLINE float SupportVertex(const Vector& dir);
};

struct NFE_ALIGN(16) Triangle
{
    Vector v0;
    Vector v1;
    Vector v2;
};

struct NFE_ALIGN(16) Ray
{
    Vector dir;
    Vector invDir;
    Vector origin;
};

struct NFE_ALIGN(16) RayBoxSegment
{
    Vector nearDist;
    Vector farDist;
};




//----------------------------------------------------------------------
//intersection tests
//----------------------------------------------------------------------
NFCOMMON_API int IntersectBoxBox(const Box& box1, const Box& box2);
NFCOMMON_API int IntersectBoxFrustum(const Box& box, const Frustum& frustum);
NFCOMMON_API int IntersectFrustumFrustum(const Frustum& f1, const Frustum& f2);

NFCOMMON_API int InsidePointBox(Box& innerBox, Box& outerBox);
NFCOMMON_API int IntersectBoxBox(Box& box1, Box& box2);
NFCOMMON_API int IntersectBoxPoint(const Box& box, const Vector& point);
NFCOMMON_API int IntersectBoxesPoint(const Box* pBoxes, Vector& point);
NFCOMMON_API int IntersectTriangleFrustum(const Triangle& tri, const Frustum& frustum);

NFCOMMON_API int IntersectRayBox(const Box& box, const Ray& ray);
NFCOMMON_API int IntersectRayBox(const Box& box, const Ray& ray, RayBoxSegment* pSegment);
NFCOMMON_API int IntersectRayTriangle(const Ray& ray, const Triangle& tri, Vector* pDist);


//
// Box class ========================================================
//

Vector Box::GetCenter() const
{
    return (min + max) * 0.5f;
}

Vector Box::GetVertex(int id) const
{
    Vector temp;
    temp.f[0] = (id & (1 << 0)) ? max.f[0] : min.f[0];
    temp.f[1] = (id & (1 << 1)) ? max.f[1] : min.f[1];
    temp.f[2] = (id & (1 << 2)) ? max.f[2] : min.f[2];
    return temp;
}

Vector Box::SupportVertex(const Vector& dir) const
{
    /*
    Vector result;
    result.f[0] = dir.f[0] > 0.0f ? max.f[0] : min.f[0];
    result.f[1] = dir.f[1] > 0.0f ? max.f[1] : min.f[1];
    result.f[2] = dir.f[2] > 0.0f ? max.f[2] : min.f[2];
    result.f[3] = 0.0f;
    return result;
    */

    __m128 compResult = _mm_cmpgt_ps(dir, _mm_setzero_ps());
    __m128 selMax = _mm_and_ps(compResult, max);
    __m128 selMin = _mm_andnot_ps(compResult, min);
    return _mm_or_ps(selMin, selMax);
}

void Box::MakeFromPoints(const Vector* pPoints, int number)
{
    if (number <= 0)
    {
        min = max = Vector();
    }
    else
    {
        min = max = pPoints[0];

        for (int i = 1; i < number; i++)
        {
            min = VectorMin(min, pPoints[i]);
            max = VectorMax(max, pPoints[i]);
        }
    }
}

float Box::SurfaceArea() const
{
    Vector size = max - min;
    return size.f[0] * (size.f[1] + size.f[2]) + size.f[1] * size.f[2];
}

float Box::Volume() const
{
    Vector size = max - min;
    return size.f[0] * size.f[1] * size.f[2];
}

// transform AABB by 4x4 matrix
NFE_INLINE Box TransformBox(const Matrix& matrix, const Box& localBox)
{
    Vector boxCenter = localBox.GetCenter();

    float x = VectorDot3f(localBox.SupportVertex(matrix.r[0]) - boxCenter, matrix.r[0]);
    float y = VectorDot3f(localBox.SupportVertex(matrix.r[1]) - boxCenter, matrix.r[1]);
    float z = VectorDot3f(localBox.SupportVertex(matrix.r[2]) - boxCenter, matrix.r[2]);
    Vector boxDim = Vector(x, y, z);

    Vector transformedBoxCenter = VectorTransform3(boxCenter, matrix);

    Box result;
    result.max = transformedBoxCenter + boxDim;
    result.min = transformedBoxCenter - boxDim;
    return result;
}



//
// Sphere class
//

float Sphere::SupportVertex(const Vector& dir)
{
    Vector pos = origin + r * dir;
    return VectorDot3(dir, pos).f[0];
}



//
// Frustum class ====================================================
//

void Frustum::CalculatePlanes()
{
    planes[0] = XPlaneFromPoints(verticies[0], verticies[1], verticies[3]); //front
    planes[1] = XPlaneFromPoints(verticies[7], verticies[5], verticies[4]); //back
    planes[2] = XPlaneFromPoints(verticies[4], verticies[0], verticies[6]); //left
    planes[3] = XPlaneFromPoints(verticies[7], verticies[3], verticies[5]); //right
    planes[4] = XPlaneFromPoints(verticies[5], verticies[1], verticies[4]); //bottom
    planes[5] = XPlaneFromPoints(verticies[6], verticies[3], verticies[7]); //top

    boundingBox.MakeFromPoints(verticies, 8);
}

float Frustum::SupportVertex(const Vector& dir)
{
    Vector d = VectorDot3(dir, verticies[0]);

    for (int i = 1; i < 8; i++)
    {
        Vector tmp_d = VectorDot3(dir, verticies[i]);
        if (_mm_comigt_ss(tmp_d, d))
        {
            d = tmp_d;
        }
    }

    return d.f[0];
}

} // namespace Math
} // namespace NFE

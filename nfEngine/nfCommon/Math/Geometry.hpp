/**
 * @file   Geometry.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Geometry objects declarations.
 */

#pragma once

namespace NFE {
namespace Math {

/**
 * Axis Aligned Box
 */
class NFE_ALIGN(16) Box
{
public:
    Vector min;
    Vector max;

    NFE_INLINE Box() : min(), max() {}
    NFE_INLINE Box(const Vector& min_, const Vector& max_) : min(min_), max(max_) {}

    // merge boxes
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

/**
 * Frustum
 */
class NFE_ALIGN(16) Frustum
{
public:
    /// planes IDs
    enum Plane
    {
        Front = 0,
        Back,
        Left,
        Right,
        Bottom,
        Top
    };

    Vector planes[6];
    Vector verticies[8];
    Box boundingBox;

    /**
     * Construct frustum of perspective view.
     * @param origin   Camera position
     * @param xAxis    Horizontal extent at distance equal to 1.
     * @param yAxis    Vertical extent at distance equal to 1.
     * @param zAxis    Normalized frustum direction.
     * @param nearDist Near plane distance.
     * @param farDist  Far plane distance.
     */
    NFE_INLINE void Construct(const Vector& origin, const Vector& xAxis, const Vector& yAxis,
                              const Vector& zAxis, float nearDist, float farDist);
    NFE_INLINE void CalculatePlanes();
    NFE_INLINE float SupportVertex(const Vector& dir) const;
};

/**
 * Sphere
 */
class NFE_ALIGN(16) Sphere
{
public:
    Vector origin;
    float r;

    NFE_INLINE Sphere() : origin(), r() {}
    NFE_INLINE Sphere(const Vector& origin, float r) : origin(origin), r(r) {}
    NFE_INLINE float SupportVertex(const Vector& dir) const;
};

class NFE_ALIGN(16) Triangle
{
public:
    Vector v0;
    Vector v1;
    Vector v2;

    NFE_INLINE Triangle() : v0(), v1(), v2() {}
    NFE_INLINE Triangle(const Vector& v0_, const Vector& v1_, const Vector& v2_)
        : v0(v0_), v1(v1_), v2(v2_) {}
};

class NFE_ALIGN(16) Ray
{
public:
    Vector dir;
    Vector invDir;
    Vector origin;
};

class NFE_ALIGN(16) RayBoxSegment
{
public:
    Vector nearDist;
    Vector farDist;
};


enum class IntersectionResult
{
    Outside   = 0, // no intersection
    Inside    = 1, // shapeA is inside shapeB
    Intersect = 2, // shapeA and shapeB intersects
};

/**
 * Finds the nearest point on a line segment to a @a point.
 * @param[in]  p1,p2 Line segment ends.
 * @param[out] out Nearest point on the segment.
 * @return     Distance to the segment.
 */
NFCOMMON_API float ClosestPointOnSegment(const Vector& p, const Vector& p1, const Vector& p2,
        Vector& out);

/**
 * Template function for intersection tests.
 */
template<typename ShapeTypeA, typename ShapeTypeB>
bool Intersect(const ShapeTypeA& shapeA, const ShapeTypeB& shapeB);

/**
 * Template function for extended intersection tests.
 * @see IntersectionResult
 */
template<typename ShapeTypeA, typename ShapeTypeB>
IntersectionResult IntersectEx(const ShapeTypeA& shapeA, const ShapeTypeB& shapeB);

/**
 * Template function for ray intersection tests.
 * @param dist Distance to the intersection.
 */
template<typename ShapeType>
bool Intersect(const Ray& ray, const ShapeType& shape, Vector& dist);


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

    float x = VectorDot3(localBox.SupportVertex(matrix.r[0]) - boxCenter, matrix.r[0])[0];
    float y = VectorDot3(localBox.SupportVertex(matrix.r[1]) - boxCenter, matrix.r[1])[0];
    float z = VectorDot3(localBox.SupportVertex(matrix.r[2]) - boxCenter, matrix.r[2])[0];
    Vector boxDim = Vector(x, y, z);

    Vector transformedBoxCenter = LinearCombination3(boxCenter, matrix);

    Box result;
    result.max = transformedBoxCenter + boxDim;
    result.min = transformedBoxCenter - boxDim;
    return result;
}



//
// Sphere class
//

float Sphere::SupportVertex(const Vector& dir) const
{
    Vector pos = origin + r * dir;
    return VectorDot3(dir, pos).f[0];
}



//
// Frustum class ====================================================
//

void Frustum::CalculatePlanes()
{
    planes[Front]  = PlaneFromPoints(verticies[0], verticies[1], verticies[3]);
    planes[Back]   = PlaneFromPoints(verticies[7], verticies[5], verticies[4]);
    planes[Left]   = PlaneFromPoints(verticies[4], verticies[0], verticies[6]);
    planes[Right]  = PlaneFromPoints(verticies[7], verticies[3], verticies[5]);
    planes[Bottom] = PlaneFromPoints(verticies[5], verticies[1], verticies[4]);
    planes[Top]    = PlaneFromPoints(verticies[6], verticies[3], verticies[7]);

    boundingBox.MakeFromPoints(verticies, 8);
}

float Frustum::SupportVertex(const Vector& dir) const
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

void Frustum::Construct(const Vector& origin, const Vector& xAxis, const Vector& yAxis,
                        const Vector& zAxis, float nearDist, float farDist)
{
    verticies[0] = origin + nearDist * (zAxis - xAxis - yAxis);
    verticies[1] = origin + nearDist * (zAxis + xAxis - yAxis);
    verticies[2] = origin + nearDist * (zAxis - xAxis + yAxis);
    verticies[3] = origin + nearDist * (zAxis + xAxis + yAxis);
    verticies[4] = origin + farDist * (zAxis - xAxis - yAxis);
    verticies[5] = origin + farDist * (zAxis + xAxis - yAxis);
    verticies[6] = origin + farDist * (zAxis - xAxis + yAxis);
    verticies[7] = origin + farDist * (zAxis + xAxis + yAxis);

    CalculatePlanes();
}

} // namespace Math
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Box class declarations.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"

namespace NFE {
namespace Math {

/**
 * Axis Aligned Box
 */
class NFE_ALIGN16 Box
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
    return VectorSelectBySign(dir, max, min);
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

/**
 * Transform AABB by a 4x4 matrix.
 */
NFCOMMON_API Box TransformBox(const Matrix& matrix, const Box& localBox);

} // namespace Math
} // namespace NFE

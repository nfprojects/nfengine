/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Box class definitions.
 */

#include "PCH.hpp"
#include "Box.hpp"


namespace NFE {
namespace Math {

Box Box::TransformBox(const Matrix& matrix, const Box& localBox)
{
    const Vector boxCenter = localBox.GetCenter();

    float x = Vector::Dot3(localBox.SupportVertex(matrix.r[0]) - boxCenter, matrix.r[0]);
    float y = Vector::Dot3(localBox.SupportVertex(matrix.r[1]) - boxCenter, matrix.r[1]);
    float z = Vector::Dot3(localBox.SupportVertex(matrix.r[2]) - boxCenter, matrix.r[2]);

    const Vector boxDim = Vector(x, y, z);
    const Vector transformedBoxCenter = matrix.LinearCombination3(boxCenter);

    return Box(transformedBoxCenter - boxDim, transformedBoxCenter + boxDim);
}

float Box::SurfaceArea() const
{
    if (IsEmpty())
    {
        return 0.0f;
    }

    const Vector size = max - min;
    return 2.0f * (size.f[0] * (size.f[1] + size.f[2]) + size.f[1] * size.f[2]);
}

float Box::Volume() const
{
    if (IsEmpty())
    {
        return 0.0f;
    }

    const Vector size = max - min;
    return size.f[0] * size.f[1] * size.f[2];
}

} // namespace Math
} // namespace NFE

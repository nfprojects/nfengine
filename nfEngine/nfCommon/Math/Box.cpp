/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Box class definitions.
 */

#include "PCH.hpp"
#include "Box.hpp"


namespace NFE {
namespace Math {

Box TransformBox(const Matrix& matrix, const Box& localBox)
{
    Vector boxCenter = localBox.GetCenter();

    float x = Vector::Dot3(localBox.SupportVertex(matrix.r[0]) - boxCenter, matrix.r[0]);
    float y = Vector::Dot3(localBox.SupportVertex(matrix.r[1]) - boxCenter, matrix.r[1]);
    float z = Vector::Dot3(localBox.SupportVertex(matrix.r[2]) - boxCenter, matrix.r[2]);
    Vector boxDim = Vector(x, y, z);

    Vector transformedBoxCenter = matrix.LinearCombination3(boxCenter);

    Box result;
    result.max = transformedBoxCenter + boxDim;
    result.min = transformedBoxCenter - boxDim;
    return result;
}

} // namespace Math
} // namespace NFE

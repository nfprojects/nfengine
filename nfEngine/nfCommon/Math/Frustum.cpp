/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Frustum class definitions.
 */

#include "PCH.hpp"
#include "Frustum.hpp"

namespace NFE {
namespace Math {

void SetupPerspective(const Matrix& matrix,
                      float nearDist, float farDist, float cutoff, float aspect,
                      Matrix& viewMatrix, Matrix& projMatrix, Frustum& frustum)
{
    // calculate view and projection matrices
    viewMatrix = MatrixLookTo(matrix.GetRow(3), matrix.GetRow(2), matrix.GetRow(1));
    projMatrix = MatrixPerspective(1.0f, cutoff, farDist, nearDist);

    Vector pos = matrix.GetRow(3) & VECTOR_MASK_XYZ;
    float scale = tanf(cutoff / 2.0f);
    Vector xAxis = aspect * scale * matrix.GetRow(0);
    Vector yAxis = scale * matrix.GetRow(1);
    Vector zAxis = scale * matrix.GetRow(2);

    frustum.Construct(pos, xAxis, yAxis, zAxis, nearDist, farDist);
}

} // namespace Math
} // namespace NFE

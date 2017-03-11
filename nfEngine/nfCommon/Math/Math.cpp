/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic math functions definitions.
 */

#include "PCH.hpp"
#include "Math.hpp"

namespace NFE {
namespace Math {

bool SolveEquationsSystem3(float M[4][3], float& x, float& y, float& z)
{
    // TODO: use SSE

    float w = M[0][0] * M[1][1] * M[2][2] +
        M[1][0] * M[2][1] * M[0][2] +
        M[2][0] * M[0][1] * M[1][2] -
        M[2][0] * M[1][1] * M[0][2] -
        M[1][0] * M[0][1] * M[2][2] -
        M[0][0] * M[2][1] * M[1][2];

    if ((w < NFE_MATH_EPSILON) && (w > -NFE_MATH_EPSILON))
        return false;

    float invW = 1.0f / w;

    float wx = M[3][0] * M[1][1] * M[2][2] +
        M[1][0] * M[2][1] * M[3][2] +
        M[2][0] * M[3][1] * M[1][2] -
        M[2][0] * M[1][1] * M[3][2] -
        M[1][0] * M[3][1] * M[2][2] -
        M[3][0] * M[2][1] * M[1][2];

    float wy = M[0][0] * M[3][1] * M[2][2] +
        M[3][0] * M[2][1] * M[0][2] +
        M[2][0] * M[0][1] * M[3][2] -
        M[2][0] * M[3][1] * M[0][2] -
        M[3][0] * M[0][1] * M[2][2] -
        M[0][0] * M[2][1] * M[3][2];

    float wz = M[0][0] * M[1][1] * M[3][2] +
        M[1][0] * M[3][1] * M[0][2] +
        M[3][0] * M[0][1] * M[1][2] -
        M[3][0] * M[1][1] * M[0][2] -
        M[1][0] * M[0][1] * M[3][2] -
        M[0][0] * M[3][1] * M[1][2];

    x = wx * invW;
    y = wy * invW;
    z = wz * invW;
    return true;
}

} // namespace Math
} // namespace NFE

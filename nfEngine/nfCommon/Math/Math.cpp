/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic math functions definitions.
 */

#include "PCH.hpp"
#include "Math.hpp"


#ifdef NFE_USE_SSE
#pragma message("INFO: Compiling nfCommon with SSE instructions")
#endif // NFE_USE_SSE

#ifdef NFE_USE_SSE4
#pragma message("INFO: Compiling nfCommon with SSE4.1 instructions")
#endif // NFE_USE_SSE4

#ifdef NFE_USE_AVX
#pragma message("INFO: Compiling nfCommon with AVX instructions")
#endif // NFE_USE_AVX

#ifdef NFE_USE_AVX2
#pragma message("INFO: Compiling nfCommon with AVX2 instructions")
#endif // NFE_USE_AVX2

#ifdef NFE_USE_FMA
#pragma message("INFO: Compiling nfCommon with FMA instructions")
#endif // NFE_USE_FMA


namespace NFE {
namespace Math {


static_assert(sizeof(Bits32) == 4, "Invalid size of 'Bits32'");
static_assert(sizeof(Bits64) == 8, "Invalid size of 'Bits64'");


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

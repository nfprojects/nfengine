/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  FPU-specific implementations of matrix operations.
 */

#pragma once

#if defined(NFE_USE_SSE)
#error "The header cannot be used when NFE_USE_SSE is defined!"
#endif

namespace NFE {
namespace Math {

Matrix MatrixTranspose(const Matrix& m)
{
    Matrix mat;
    mat.r[0] = Vector(m.r[0].f[0], m.r[1].f[0], m.r[2].f[0], m.r[3].f[0]);
    mat.r[1] = Vector(m.r[0].f[1], m.r[1].f[1], m.r[2].f[1], m.r[3].f[1]);
    mat.r[2] = Vector(m.r[0].f[2], m.r[1].f[2], m.r[2].f[2], m.r[3].f[2]);
    mat.r[3] = Vector(m.r[0].f[3], m.r[1].f[3], m.r[2].f[3], m.r[3].f[3]);
    return mat;
}

} // namespace Math
} // namespace NFE

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

Matrix Matrix::Transposed() const
{
    Matrix mat;
    mat.r[0] = Vector(r[0].f[0], r[1].f[0], r[2].f[0], r[3].f[0]);
    mat.r[1] = Vector(r[0].f[1], r[1].f[1], r[2].f[1], r[3].f[1]);
    mat.r[2] = Vector(r[0].f[2], r[1].f[2], r[2].f[2], r[3].f[2]);
    mat.r[3] = Vector(r[0].f[3], r[1].f[3], r[2].f[3], r[3].f[3]);
    return mat;
}

Matrix& Matrix::Transpose()
{
    *this = Transposed();
    return *this;
}

} // namespace Math
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  SSE-specific implementations of matrix operations.
 */

#pragma once

#if !defined(NFE_USE_SSE)
#error "NFE_USE_SSE must be defined when including this header"
#endif

namespace NFE {
namespace Math {

Matrix MatrixTranspose(const Matrix& m)
{
    Vector row0 = m.r[0];
    Vector row1 = m.r[1];
    Vector row2 = m.r[2];
    Vector row3 = m.r[3];

    _MM_TRANSPOSE4_PS(row0, row1, row2, row3);

    return Matrix(row0, row1, row2, row3);
}

} // namespace Math
} // namespace NFE

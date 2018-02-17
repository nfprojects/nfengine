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

Matrix4& Matrix4::Transpose()
{
    Vector4& row0 = r[0];
    Vector4& row1 = r[1];
    Vector4& row2 = r[2];
    Vector4& row3 = r[3];

    _MM_TRANSPOSE4_PS(row0, row1, row2, row3);

    return *this;
}

Matrix4 Matrix4::Transposed() const
{
    Vector4 row0 = r[0];
    Vector4 row1 = r[1];
    Vector4 row2 = r[2];
    Vector4 row3 = r[3];

    _MM_TRANSPOSE4_PS(row0, row1, row2, row3);

    return Matrix4(row0, row1, row2, row3);
}

} // namespace Math
} // namespace NFE

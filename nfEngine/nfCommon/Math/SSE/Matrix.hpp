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
    Vector tmp0 = _mm_shuffle_ps(m[0], m[1], 0x44);
    Vector tmp2 = _mm_shuffle_ps(m[0], m[1], 0xEE);
    Vector tmp1 = _mm_shuffle_ps(m[2], m[3], 0x44);
    Vector tmp3 = _mm_shuffle_ps(m[2], m[3], 0xEE);
    return Matrix(_mm_shuffle_ps(tmp0, tmp1, 0x88),
                  _mm_shuffle_ps(tmp0, tmp1, 0xDD),
                  _mm_shuffle_ps(tmp2, tmp3, 0x88),
                  _mm_shuffle_ps(tmp2, tmp3, 0xDD));
}

} // namespace Math
} // namespace NFE

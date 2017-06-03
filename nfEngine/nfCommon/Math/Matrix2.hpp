/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix2 class declaration.
 */

#pragma once

#include "Math.hpp"
#include "Float2.hpp"

#include <initializer_list>


namespace NFE {
namespace Math {

/**
 * 2x2 matrix.
 * Unaligned, efficient in terms of memory usage.
 */
class Matrix2
{
public:
    union
    {
        Float2 r[2];
        float f[4];
        float m[2][2];
    };

    NFE_INLINE constexpr Matrix2();
    NFE_INLINE constexpr explicit Matrix2(const Float2& r0, const Float2& r1);
    NFE_INLINE explicit Matrix2(const std::initializer_list<float>& list);

    /**
     * Access element (read-only).
     */
    NFE_INLINE const Float2& GetRow(int i) const;
    NFE_INLINE const Float2& operator[] (int i) const;

    /**
     * Access element (read-write).
     */
    NFE_INLINE Float2& GetRow(int i);
    NFE_INLINE Float2& operator[] (int i);

    /**
     * Offset matrix element by the same value.
     */
    NFE_INLINE Matrix2 operator + (const Matrix2& b) const;
    NFE_INLINE Matrix2 operator - (const Matrix2& b) const;
    NFE_INLINE Matrix2& operator += (const Matrix2& b);
    NFE_INLINE Matrix2& operator -= (const Matrix2& b);

    /**
     * Matrix2 scaling.
     */
    NFE_INLINE Matrix2 operator * (float b) const;
    NFE_INLINE Matrix2 operator / (float b) const;
    NFE_INLINE Matrix2& operator *= (float b);
    NFE_INLINE Matrix2& operator /= (float b);

    NFE_INLINE static Matrix2 Abs(const Matrix2& m);

    /**
     * Matrix2-matrix multiplication.
     */
    NFCOMMON_API Matrix2 operator * (const Matrix2& b) const;
    NFE_INLINE Matrix2& operator *= (const Matrix2& b);

    /**
     * Returns true if all the corresponding elements are (exactly) equal.
     */
    NFE_INLINE constexpr bool operator == (const Matrix2& b) const;

    /**
     * Calculate matrix determinant.
     */
    NFE_INLINE constexpr float Det() const;

    NFE_INLINE constexpr Matrix2 Transposed() const;
    NFE_INLINE Matrix2& Transpose();

    /**
     * Calculate matrix inverse.
     * @param[out] outInverted Inverted matrix.
     * @return      True if matrix is invertible.
     */
    NFCOMMON_API bool Invert(Matrix2& outInverted) const;

    /**
     * Multiply a 2D vector by a 2x2 matrix.
     * @return  a.x * m.r[0] + a.y * m.r[1]
     */
    NFE_INLINE Float2 LinearCombination(const Float2& a) const;

    /**
     * Check if two matrices are (almost) equal.
     */
    NFCOMMON_API static bool Equal(const Matrix2& m1, const Matrix2& m2, float epsilon);

    /**
     * Create 2D rotation matrix.
     * @param   angle   Rotation angle in radians.
     */
    NFCOMMON_API static Matrix2 MakeRotation(float angle);
};

/**
 * Alias for Matrix2::LinearCombination()
 */
NFE_INLINE Float2 operator * (const Float2& vector, const Matrix2& m);


} // namespace Math
} // namespace NFE


#include "Matrix2Impl.hpp"

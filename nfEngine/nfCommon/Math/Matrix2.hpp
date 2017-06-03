/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix2 class declaration.
 */

#pragma once

#include "Math.hpp"

#include <initializer_list>


namespace NFE {
namespace Math {

/**
 * 2x2 matrix
 */
class Matrix2 final
{
public:
    union
    {
        Float2 r[2];
        float f[4];
        float m[2][2];
    };

    /**
     * Default constructor - create identity matrix.
     */
    NFE_INLINE constexpr Matrix2();

    /**
     * Create matrix from rows.
     */
    NFE_INLINE constexpr explicit Matrix2(const Float2& r0, const Float2& r1);

    /**
     * Create matrix from element values.
     */
    NFE_INLINE constexpr explicit Matrix2(float r0c0, float r0c1, float r1c0, float r1c1);
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
    NFE_INLINE Matrix2 operator+ (const Matrix2& b) const;
    NFE_INLINE Matrix2 operator- (const Matrix2& b) const;
    NFE_INLINE Matrix2& operator+= (const Matrix2& b);
    NFE_INLINE Matrix2& operator-= (const Matrix2& b);

    /**
     * Matrix2 scaling.
     */
    NFE_INLINE Matrix2 operator* (float b) const;
    NFE_INLINE Matrix2 operator/ (float b) const;
    NFE_INLINE Matrix2& operator*= (float b);
    NFE_INLINE Matrix2& operator/= (float b);

    /**
     * Matrix2-matrix multiplication.
     */
    NFCOMMON_API Matrix2 operator* (const Matrix2& b) const;
    NFCOMMON_API Matrix2& operator*= (const Matrix2& b);

    /**
     * Returns true if all the corresponding elements are (exactly) equal.
     */
    NFE_INLINE bool operator== (const Matrix2& b) const;

    /**
     * Calculate matrix inverse.
     */
    NFCOMMON_API Matrix2 Inverted() const;

    /**
     * Calculate matrix inverse.
     */
    NFE_INLINE Matrix2& Invert();

    /**
     * Multiply a 2D vector by a 2x2 matrix.
     * @return  a.x * m.r[0] + a.y * m.r[1]
     */
    NFE_INLINE Float2 LinearCombination(const Float2& a) const;

    /**
     * Calculate matrix containing absolute values of another.
     */
    NFE_INLINE static Matrix2 Abs(const Matrix2& m);

    /**
     * Check if two matrices are (almost) equal.
     */
    NFE_INLINE static bool Equal(const Matrix2& m1, const Matrix2& m2, float epsilon);

    /**
     * Calculate transpose matrix.
     */
    NFE_INLINE Matrix2& Transpose();

    /**
     * Calculate transpose matrix.
     */
    NFE_INLINE Matrix2 Transposed() const;
};


} // namespace Math
} // namespace NFE


#include "Matrix2Impl.hpp"

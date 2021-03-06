/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix3 class declaration.
 */

#pragma once

#include "Math.hpp"
#include "Vec3f.hpp"

#include <initializer_list>


namespace NFE {
namespace Math {

/**
 * 3x3 matrix
 *
 * Unaligned, efficient in terms of memory usage.
 */
class Matrix3 final
{
public:
    union
    {
        Vec3f r[3];
        float f[9];
        float m[3][3];
    };

    NFE_INLINE constexpr Matrix3();
    NFE_INLINE constexpr explicit Matrix3(const Vec3f& r0, const Vec3f& r1, const Vec3f& r2);
    NFE_INLINE explicit Matrix3(const std::initializer_list<float>& list);

    /**
     * Access element (read-only).
     */
    NFE_INLINE const Vec3f& GetRow(int i) const;
    NFE_INLINE const Vec3f& operator[] (int i) const;

    /**
     * Access element (read-write).
     */
    NFE_INLINE Vec3f& GetRow(int i);
    NFE_INLINE Vec3f& operator[] (int i);

    /**
     * Offset matrix element by the same value.
     */
    NFE_INLINE Matrix3 operator + (const Matrix3& b) const;
    NFE_INLINE Matrix3 operator - (const Matrix3& b) const;
    NFE_INLINE Matrix3& operator += (const Matrix3& b);
    NFE_INLINE Matrix3& operator -= (const Matrix3& b);

    /**
     * Matrix3 scaling.
     */
    NFE_INLINE Matrix3 operator * (float b) const;
    NFE_INLINE Matrix3 operator / (float b) const;
    NFE_INLINE Matrix3& operator *= (float b);
    NFE_INLINE Matrix3& operator /= (float b);

    NFE_INLINE static Matrix3 Abs(const Matrix3& m);

    /**
     * Matrix3-matrix multiplication.
     */
    NFCOMMON_API Matrix3 operator * (const Matrix3& b) const;
    NFCOMMON_API Matrix3& operator *= (const Matrix3& b);

    /**
     * Returns true if all the corresponding elements are (exactly) equal.
     */
    NFE_INLINE constexpr bool operator == (const Matrix3& b) const;

    /**
     * Calculate matrix determinant.
     */
    NFE_INLINE constexpr float Det() const;

    NFE_INLINE constexpr Matrix3 Transposed() const;
    NFE_INLINE Matrix3& Transpose();

    /**
     * Calculate matrix inverse.
     * @param[out] outInverted Inverted matrix.
     * @return      True if matrix is invertible.
     */
    NFCOMMON_API bool Invert(Matrix3& outInverted) const;

    /**
     * Multiply a 3D vector by a 3x3 matrix.
     * @return  a.x * m.r[0] + a.y * m.r[1]
     */
    NFE_INLINE Vec3f LinearCombination(const Vec3f& a) const;

    /**
     * Multiply a 2D vector by a 3x3 matrix (affine transformation).
     * @return  a.x * m.r[0] + a.y * m.r[1] + m.r[2].xy
     */
    NFE_INLINE Vec2f AffineTransform2(const Vec2f& a) const;

    /**
     * Check if two matrices are (almost) equal.
     */
    NFCOMMON_API static bool Equal(const Matrix3& m1, const Matrix3& m2, float epsilon);
};

/**
 * Alias for Matrix3::LinearCombination()
 */
NFE_INLINE Vec3f operator* (const Vec3f& vector, const Matrix3& m);


} // namespace Math
} // namespace NFE


#include "Matrix3Impl.hpp"

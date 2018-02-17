/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix4 class declaration.
 */

#pragma once

#include "Math.hpp"
#include "Vector4.hpp"
#include "Matrix2.hpp"
#include "Matrix3.hpp"

#include <initializer_list>

namespace NFE {
namespace Math {

/**
 * 4x4 matrix
 */
class NFE_ALIGN(16) Matrix4 final
{
public:
    union
    {
        Vector4 r[4];   //< rows
        float f[16];
        float m[4][4];
    };

    /**
     * Default constructor - create identity matrix.
     */
    NFE_INLINE Matrix4();

    /**
     * Create matrix from rows.
     */
    NFE_INLINE explicit Matrix4(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3);

    /**
     * Create matrix from element values.
     */
    NFE_INLINE explicit Matrix4(const std::initializer_list<float>& list);

    /**
     * Access element (read-only).
     */
    NFE_INLINE const Vector4& GetRow(int i) const;
    NFE_INLINE const Vector4& operator[] (int i) const;

    /**
     * Access element (read-write).
     */
    NFE_INLINE Vector4& GetRow(int i);
    NFE_INLINE Vector4& operator[] (int i);

    /**
     * Extract 2x2 matrix comprised of 2 first element of 3 first rows.
     */
    NFE_INLINE Matrix2 ToMatrix2() const;

    /**
     * Extract 3x3 matrix minor comprised of 3 first element of 3 first rows.
     */
    NFE_INLINE Matrix3 ToMatrix3() const;

    /**
     * Offset matrix element by the same value.
     */
    NFE_INLINE Matrix4 operator+ (const Matrix4& b) const;
    NFE_INLINE Matrix4 operator- (const Matrix4& b) const;
    NFE_INLINE Matrix4& operator+= (const Matrix4& b);
    NFE_INLINE Matrix4& operator-= (const Matrix4& b);

    /**
     * Matrix scaling.
     */
    NFE_INLINE Matrix4 operator* (float b) const;
    NFE_INLINE Matrix4 operator/ (float b) const;
    NFE_INLINE Matrix4& operator*= (float b);
    NFE_INLINE Matrix4& operator/= (float b);

    /**
     * Matrix-matrix multiplication.
     */
    NFCOMMON_API Matrix4 operator* (const Matrix4& b) const;
    NFCOMMON_API Matrix4& operator*= (const Matrix4& b);

    /**
     * Returns true if all the corresponding elements are (exactly) equal.
     */
    NFE_INLINE bool operator== (const Matrix4& b) const;

    /**
     * Calculate matrix inverse.
     */
    NFCOMMON_API Matrix4 Inverted() const;

    /**
     * Calculate matrix inverse.
     */
    NFE_INLINE Matrix4& Invert();

    /**
     * Create rotation matrix.
     * @param normalAxis Normalized axis.
     * @param angle Rotation angle (in radians).
     */
    NFCOMMON_API static Matrix4 MakeRotationNormal(const Vector4& normalAxis, float angle);

    /**
     * Create perspective projection matrix.
     * @param aspect Aspect ratio.
     * @param fovY Vertical field of view angle.
     * @param farZ,nearZ Far and near distances.
     */
    NFCOMMON_API static Matrix4 MakePerspective(float aspect, float fovY, float farZ, float nearZ);

    /**
     * Create orthographic projection matrix.
     * @param left,right X-axis boundaries.
     * @param bottom,top Y-axis boundaries.
     * @param zNear,zFar Z-axis boundaries.
     */
    NFCOMMON_API static Matrix4 MakeOrtho(float left, float right, float bottom, float top, float zNear, float zFar);

    /**
     * Create scaling matrix
     * @param scale Scaling factor (only XYZ components are taken into account).
     */
    NFCOMMON_API static Matrix4 MakeScaling(const Vector4& scale);

    /**
     * Create view matrix.
     * @param eyePosition  Observer's position.
     * @param eyeDirection Observer's direction.
     * @param upDirection  "Up" vector.
     */
    NFCOMMON_API static Matrix4 MakeLookTo(const Vector4& eyePosition, const Vector4& eyeDirection, const Vector4& upDirection);

    /**
     * Multiply a 3D vector by a 4x4 matrix (affine transform).
     * Equivalent of a[0] * m.r[0] + a[1] * m.r[1] + a[2] * m.r[2] + m.r[3]
     */
    NFE_INLINE Vector4 LinearCombination3(const Vector4& a) const;

    /**
     * Multiply a 4D vector by a 4x4 matrix.
     * Equivalent of a[0] * m.r[0] + a[1] * m.r[1] + a[2] * m.r[2] + a[3] * m.r[3]
     */
    NFE_INLINE Vector4 LinearCombination4(const Vector4& a) const;

    /**
     * Calculate matrix containing absolute values of another.
     */
    NFE_INLINE static Matrix4 Abs(const Matrix4& m);

    /**
     * Check if two matrices are (almost) equal.
     */
    NFE_INLINE static bool Equal(const Matrix4& m1, const Matrix4& m2, float epsilon);

    /**
     * Create matrix representing a translation by 3D vector.
     */
    NFE_INLINE static Matrix4 MakeTranslation3(const Vector4& pos);

    /**
     * Calculate transpose matrix.
     */
    NFE_INLINE Matrix4& Transpose();

    /**
     * Calculate transpose matrix.
     */
    NFE_INLINE Matrix4 Transposed() const;
};


/**
 * Alias of @p Matrix4::LinearCombination4 function.
 */
NFE_INLINE Vector4 operator* (const Vector4& vector, const Matrix4& m);


} // namespace Math
} // namespace NFE


// architecture-independent inline function definitions go here:
#include "Matrix4Impl.hpp"


// include architecture-specific implementations
#ifdef NFE_USE_SSE
#include "SSE/Matrix4.hpp"
#else
#include "FPU/Matrix4.hpp"
#endif

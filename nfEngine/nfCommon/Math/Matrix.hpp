/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix class declaration.
 */

#pragma once

#include "Math.hpp"
#include "Vector4.hpp"

#include <initializer_list>

namespace NFE {
namespace Math {

/**
 * 4x4 matrix
 */
class NFE_ALIGN(16) Matrix final
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
    NFE_INLINE Matrix();

    /**
     * Create matrix from rows.
     */
    NFE_INLINE explicit Matrix(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3);

    /**
     * Create matrix from element values.
     */
    NFE_INLINE explicit Matrix(const std::initializer_list<float>& list);

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
     * Offset matrix element by the same value.
     */
    NFE_INLINE Matrix operator+ (const Matrix& b) const;
    NFE_INLINE Matrix operator- (const Matrix& b) const;
    NFE_INLINE Matrix& operator+= (const Matrix& b);
    NFE_INLINE Matrix& operator-= (const Matrix& b);

    /**
     * Matrix scaling.
     */
    NFE_INLINE Matrix operator* (float b) const;
    NFE_INLINE Matrix operator/ (float b) const;
    NFE_INLINE Matrix& operator*= (float b);
    NFE_INLINE Matrix& operator/= (float b);

    /**
     * Matrix-matrix multiplication.
     */
    NFCOMMON_API Matrix operator* (const Matrix& b) const;
    NFCOMMON_API Matrix& operator*= (const Matrix& b);

    /**
     * Returns true if all the corresponding elements are (exactly) equal.
     */
    NFE_INLINE bool operator== (const Matrix& b) const;

    /**
     * Calculate matrix inverse.
     */
    NFCOMMON_API Matrix Inverted() const;

    /**
     * Calculate matrix inverse.
     */
    NFE_INLINE Matrix& Invert();

    /**
     * Create rotation matrix.
     * @param normalAxis Normalized axis.
     * @param angle Rotation angle (in radians).
     */
    NFCOMMON_API static Matrix MakeRotationNormal(const Vector4& normalAxis, float angle);

    /**
     * Create perspective projection matrix.
     * @param aspect Aspect ratio.
     * @param fovY Vertical field of view angle.
     * @param farZ,nearZ Far and near distances.
     */
    NFCOMMON_API static Matrix MakePerspective(float aspect, float fovY, float farZ, float nearZ);

    /**
     * Create orthographic projection matrix.
     * @param left,right X-axis boundaries.
     * @param bottom,top Y-axis boundaries.
     * @param zNear,zFar Z-axis boundaries.
     */
    NFCOMMON_API static Matrix MakeOrtho(float left, float right, float bottom, float top, float zNear, float zFar);

    /**
     * Create scaling matrix
     * @param scale Scaling factor (only XYZ components are taken into account).
     */
    NFCOMMON_API static Matrix MakeScaling(const Vector4& scale);

    /**
     * Create view matrix.
     * @param eyePosition  Observer's position.
     * @param eyeDirection Observer's direction.
     * @param upDirection  "Up" vector.
     */
    NFCOMMON_API static Matrix MakeLookTo(const Vector4& eyePosition, const Vector4& eyeDirection, const Vector4& upDirection);

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
    NFE_INLINE static Matrix Abs(const Matrix& m);

    /**
     * Check if two matrices are (almost) equal.
     */
    NFE_INLINE static bool Equal(const Matrix& m1, const Matrix& m2, float epsilon);

    /**
     * Create matrix representing a translation by 3D vector.
     */
    NFE_INLINE static Matrix MakeTranslation3(const Vector4& pos);

    /**
     * Calculate transpose matrix.
     */
    NFE_INLINE Matrix& Transpose();

    /**
     * Calculate transpose matrix.
     */
    NFE_INLINE Matrix Transposed() const;
};


/**
 * Alias of @p Matrix::LinearCombination4 function.
 */
NFE_INLINE Vector4 operator* (const Vector4& vector, const Matrix& m);


} // namespace Math
} // namespace NFE


// architecture-independent inline function definitions go here:
#include "MatrixImpl.hpp"


// include architecture-specific implementations
#ifdef NFE_USE_SSE
#include "SSE/Matrix.hpp"
#else
#include "FPU/Matrix.hpp"
#endif

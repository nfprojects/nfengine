/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix class declaration.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"

namespace NFE {
namespace Math {

/**
 * 4x4 matrix
 */
class NFE_ALIGN16 Matrix
{
public:
    union
    {
        Vector r[4];   //< rows
        float f[16];
        float m[4][4];
    };

    NFE_INLINE Vector& GetRow(int i)
    {
        return reinterpret_cast<Vector&>(r[i]);
    }

    NFE_INLINE const Vector& GetRow(int i) const
    {
        return reinterpret_cast<const Vector&>(r[i]);
    }

    NFE_INLINE Vector& operator[] (int i)
    {
        return reinterpret_cast<Vector&>(r[i]);
    }

    NFE_INLINE const Vector& operator[] (int i) const
    {
        return reinterpret_cast<const Vector&>(r[i]);
    }

    /**
     * Default constructor - create identity matrix.
     */
    NFE_INLINE Matrix()
    {
        r[0] = VECTOR_IDENTITY_ROW_0;
        r[1] = VECTOR_IDENTITY_ROW_1;
        r[2] = VECTOR_IDENTITY_ROW_2;
        r[3] = VECTOR_IDENTITY_ROW_3;
    }

    /**
     * Create matrix from rows.
     */
    NFE_INLINE Matrix(const Vector& r0, const Vector& r1, const Vector& r2, const Vector& r3)
    {
        r[0] = r0;
        r[1] = r1;
        r[2] = r2;
        r[3] = r3;
    }

    /**
     * Create matrix from element values.
     */
    NFE_INLINE Matrix(float e0, float e1, float e2, float e3,
                      float e4, float e5, float e6, float e7,
                      float e8, float e9, float e10, float e11,
                      float e12, float e13, float e14, float e15)
    {
        f[0] = e0;
        f[1] = e1;
        f[2] = e2;
        f[3] = e3;
        f[4] = e4;
        f[5] = e5;
        f[6] = e6;
        f[7] = e7;
        f[8] = e8;
        f[9] = e9;
        f[10] = e10;
        f[11] = e11;
        f[12] = e12;
        f[13] = e13;
        f[14] = e14;
        f[15] = e15;
    }

    NFE_INLINE Matrix operator+ (const Matrix& b) const
    {
        return Matrix(r[0] + b.r[0], r[1] + b.r[1], r[2] + b.r[2], r[3] + b.r[3]);
    }

    NFE_INLINE Matrix operator- (const Matrix& b) const
    {
        return Matrix(r[0] - b.r[0], r[1] - b.r[1], r[2] - b.r[2], r[3] - b.r[3]);
    }

    NFE_INLINE Matrix& operator+= (const Matrix& b)
    {
        r[0] += b.r[0];
        r[1] += b.r[1];
        r[2] += b.r[2];
        r[3] += b.r[3];
        return *this;
    }

    NFE_INLINE Matrix& operator-= (const Matrix& b)
    {
        r[0] -= b.r[0];
        r[1] -= b.r[1];
        r[2] -= b.r[2];
        r[3] -= b.r[3];
        return *this;
    }

    NFE_INLINE Matrix operator* (float b) const
    {
        return Matrix(r[0] * b, r[1] * b, r[2] * b, r[3] * b);
    }

    NFE_INLINE Matrix operator/ (float b) const
    {
        return Matrix(r[0] / b, r[1] / b, r[2] / b, r[3] / b);
    }

    NFE_INLINE Matrix& operator*= (float b)
    {
        r[0] *= b;
        r[1] *= b;
        r[2] *= b;
        r[3] *= b;
        return *this;
    }

    NFE_INLINE Matrix& operator/= (float b)
    {
        r[0] /= b;
        r[1] /= b;
        r[2] /= b;
        r[3] /= b;
        return *this;
    }

    NFCOMMON_API Matrix operator* (const Matrix& b) const;
    NFCOMMON_API Matrix& operator*= (const Matrix& b);

    /**
     * Returns true if all the corresponding elements are equal.
     */
    NFE_INLINE bool operator== (const Matrix& b) const
    {
        int tmp0 = r[0] == b.r[0];
        int tmp1 = r[1] == b.r[1];
        int tmp2 = r[2] == b.r[2];
        int tmp3 = r[3] == b.r[3];
        return (tmp0 && tmp1) & (tmp2 && tmp3);
    }
};

/**
 * Calculate matrix inverse.
 */
NFCOMMON_API Matrix MatrixInverse(const Matrix& m);

/**
 * Create rotation matrix.
 * @param normalAxis Normalized axis.
 * @param angle Rotation angle (in radians).
 */
NFCOMMON_API Matrix MatrixRotationNormal(const Vector& normalAxis, float angle);

/**
 * Create perspective projection matrix.
 * @param aspect Aspect ratio.
 * @param fovY Vertical field of view angle.
 * @param farZ,nearZ Far and near distances.
 */
NFCOMMON_API Matrix MatrixPerspective(float aspect, float fovY, float farZ, float nearZ);

/**
 * Create orthographic projection matrix.
 * @param left,right X-axis boundaries.
 * @param bottom,top Y-axis boundaries.
 * @param zNear,zFar Z-axis boundaries.
 */
NFCOMMON_API Matrix MatrixOrtho(float left, float right, float bottom, float top,
                                float zNear, float zFar);

/**
 * Create scaling matrix
 * @param scale Scaling factor (only XYZ components are taken into account).
 */
NFCOMMON_API Matrix MatrixScaling(const Vector& scale);

/**
 * Create view matrix.
 * @param eyePosition  Observer's position.
 * @param eyeDirection Observer's direction.
 * @param upDirection  "Up" vector.
 */
NFCOMMON_API Matrix MatrixLookTo(const Vector& eyePosition, const Vector& eyeDirection,
                                 const Vector& upDirection);

/**
 * Multiply a 4D vector by a 4x4 matrix.
 * Equivalent of a[0] * m.r[0] + a[1] * m.r[1] + a[2] * m.r[2] + a[3] * m.r[3]
 */
NFE_INLINE Vector LinearCombination(const Vector& a, const Matrix& m)
{
    Vector tmp0 = a.SplatX() * m.r[0];
    Vector tmp1 = a.SplatY() * m.r[1];
    Vector tmp2 = a.SplatZ() * m.r[2];
    Vector tmp3 = a.SplatW() * m.r[3];
    return (tmp0 + tmp1) + (tmp2 + tmp3);
}

/**
 * Alias of @p LinearCombination function.
 */
NFE_INLINE Vector operator* (const Vector& a, const Matrix& m)
{
    return LinearCombination(a, m);
}

/**
 * Multiply a 3D vector by a 4x4 matrix (affine transform).
 * Equivalent of a[0] * m.r[0] + a[1] * m.r[1] + a[2] * m.r[2] + m.r[3]
 */
NFE_INLINE Vector LinearCombination3(const Vector& a, const Matrix& m)
{
    Vector tmp0 = a.SplatX() * m.r[0];
    Vector tmp1 = a.SplatY() * m.r[1];
    Vector tmp2 = a.SplatZ() * m.r[2];
    return (tmp0 + tmp1) + (tmp2 + m[3]);
}

/**
 * Calculate transpose matrix.
 */
NFE_INLINE Matrix MatrixTranspose(const Matrix& m)
{
#ifdef NFE_USE_SSE
    Vector tmp0 = _mm_shuffle_ps(m[0], m[1], 0x44);
    Vector tmp2 = _mm_shuffle_ps(m[0], m[1], 0xEE);
    Vector tmp1 = _mm_shuffle_ps(m[2], m[3], 0x44);
    Vector tmp3 = _mm_shuffle_ps(m[2], m[3], 0xEE);
    return Matrix(_mm_shuffle_ps(tmp0, tmp1, 0x88),
                  _mm_shuffle_ps(tmp0, tmp1, 0xDD),
                  _mm_shuffle_ps(tmp2, tmp3, 0x88),
                  _mm_shuffle_ps(tmp2, tmp3, 0xDD));
#else
    Matrix mat;
    mat.r[0] = Vector(m.r[0].f[0], m.r[1].f[0], m.r[2].f[0], m.r[3].f[0]);
    mat.r[1] = Vector(m.r[0].f[1], m.r[1].f[1], m.r[2].f[1], m.r[3].f[1]);
    mat.r[2] = Vector(m.r[0].f[2], m.r[1].f[2], m.r[2].f[2], m.r[3].f[2]);
    mat.r[3] = Vector(m.r[0].f[3], m.r[1].f[3], m.r[2].f[3], m.r[3].f[3]);
    return mat;
#endif
}

/**
 * Calculate matrix containing absolute values of another.
 */
NFE_INLINE Matrix MatrixAbs(const Matrix& m)
{
    return Matrix(VectorAbs(m[0]),
                  VectorAbs(m[1]),
                  VectorAbs(m[2]),
                  VectorAbs(m[3]));
}

/**
 * Check if two matrices are (almost) equal.
 */
NFE_INLINE bool MatrixEqual(const Matrix& m1, const Matrix& m2, float epsilon)
{
    Matrix diff = MatrixAbs(m1 - m2);
    Vector epsilonV = VectorSplat(epsilon);
    return ((diff[0] < epsilonV) && (diff[1] < epsilonV)) &&
           ((diff[2] < epsilonV) && (diff[3] < epsilonV));
}


/**
 * Create matrix representing a translation by 3D vector.
 */
NFE_INLINE Matrix MatrixTranslation3(const Vector& pos)
{
    Matrix m;
    m.r[0] = VECTOR_IDENTITY_ROW_0;
    m.r[1] = VECTOR_IDENTITY_ROW_1;
    m.r[2] = VECTOR_IDENTITY_ROW_2;
    m.r[3] = Vector(pos.f[0], pos.f[1], pos.f[2], 1.0f);
    return m;
}

} // namespace Math
} // namespace NFE

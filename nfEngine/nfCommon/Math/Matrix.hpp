/**
 * @file   Matrix.h
 * @brief  Matrix class and helper functions declaration.
 */

#pragma once

namespace NFE {
namespace Math {

/**
 * 4x4 matrix
 */
NFE_ALIGN(16)
struct Matrix
{
    union
    {
        __m128 r[4];   //< rows
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
        r[0] = g_IdentityR0;
        r[1] = g_IdentityR1;
        r[2] = g_IdentityR2;
        r[3] = g_IdentityR3;
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

    NFE_INLINE Matrix operator+ (const Matrix& b) const
    {
        return Matrix(_mm_add_ps(r[0], b.r[0]),
                      _mm_add_ps(r[1], b.r[1]),
                      _mm_add_ps(r[2], b.r[2]),
                      _mm_add_ps(r[3], b.r[3]));
    }

    NFE_INLINE Matrix operator- (const Matrix& b) const
    {
        return Matrix(_mm_sub_ps(r[0], b.r[0]),
                      _mm_sub_ps(r[1], b.r[1]),
                      _mm_sub_ps(r[2], b.r[2]),
                      _mm_sub_ps(r[3], b.r[3]));
    }

    NFE_INLINE Matrix& operator+= (const Matrix& b)
    {
        r[0] = _mm_add_ps(r[0], b.r[0]);
        r[1] = _mm_add_ps(r[1], b.r[1]);
        r[2] = _mm_add_ps(r[2], b.r[2]);
        r[3] = _mm_add_ps(r[3], b.r[3]);
        return *this;
    }

    NFE_INLINE Matrix& operator-= (const Matrix& b)
    {
        r[0] = _mm_sub_ps(r[0], b.r[0]);
        r[1] = _mm_sub_ps(r[1], b.r[1]);
        r[2] = _mm_sub_ps(r[2], b.r[2]);
        r[3] = _mm_sub_ps(r[3], b.r[3]);
        return *this;
    }

    NFCOMMON_API Matrix operator* (const Matrix& b) const;
    NFCOMMON_API Matrix& operator*= (const Matrix& b);

    /**
     * Returns true if all the corresponding elements are equal.
     */
    NFE_INLINE bool operator== (const Matrix& b)
    {
        return (_mm_movemask_ps(_mm_cmpeq_ps(r[0], b.r[0])) &
                _mm_movemask_ps(_mm_cmpeq_ps(r[1], b.r[1])) &
                _mm_movemask_ps(_mm_cmpeq_ps(r[2], b.r[2])) &
                _mm_movemask_ps(_mm_cmpeq_ps(r[3], b.r[3]))) == 0xF;
    }
};


/**
 * Calculate matrix inverse.
 */
NFCOMMON_API Matrix MatrixInverse(const Matrix& M);

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
NFCOMMON_API Matrix MatrixOrtho(float left, float right, float bottom, float top, float zNear,
                                float zFar);

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
    Vector result = _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 0)), m.r[0]);
    result += _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1, 1, 1, 1)), m.r[1]);
    result += _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(2, 2, 2, 2)), m.r[2]);
    result += _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 3, 3, 3)), m.r[3]);
    return result;
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
    Vector result = _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 0)), m.r[0]);
    result += _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1, 1, 1, 1)), m.r[1]);
    result += _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(2, 2, 2, 2)), m.r[2]);
    result += m.r[3];
    return result;
}

/**
 * Calculate transpose matrix.
 */
NFE_INLINE Matrix MatrixTranspose(const Matrix& M)
{   
    Vector tmp0 = _mm_shuffle_ps(M.r[0], M.r[1], 0x44);
    Vector tmp2 = _mm_shuffle_ps(M.r[0], M.r[1], 0xEE);
    Vector tmp1 = _mm_shuffle_ps(M.r[2], M.r[3], 0x44);
    Vector tmp3 = _mm_shuffle_ps(M.r[2], M.r[3], 0xEE);

    return Matrix(_mm_shuffle_ps(tmp0, tmp1, 0x88),
                  _mm_shuffle_ps(tmp0, tmp1, 0xDD),
                  _mm_shuffle_ps(tmp2, tmp3, 0x88),
                  _mm_shuffle_ps(tmp2, tmp3, 0xDD));
}

/**
 * Create matrix representing a translation by 3D vector.
 */
NFE_INLINE Matrix MatrixTranslation3(const Vector& pos)
{
    Matrix M;
    M.r[0] = g_IdentityR0;
    M.r[1] = g_IdentityR1;
    M.r[2] = g_IdentityR2;
    M.r[3] = Vector(pos.f[0], pos.f[1], pos.f[2], 1.0f);
    return M;
}

} // namespace Math
} // namespace NFE

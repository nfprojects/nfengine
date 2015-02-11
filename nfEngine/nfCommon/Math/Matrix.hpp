/**
 * @file   Matrix.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix class declaration.
 */

#pragma once

namespace NFE {
namespace Math {

class NFE_ALIGN(16) Matrix
{
public:
    union
    {
        __m128 r[4];
        float f[16];
        float m[4][4];
    };

    Vector GetRow(int i)
    {
        return Vector(r[i]);
    }

    // default constructor - create identity matrix
    Matrix()
    {
        r[0] = g_IdentityR0;
        r[1] = g_IdentityR1;
        r[2] = g_IdentityR2;
        r[3] = g_IdentityR3;
    }
};


NFE_INLINE Matrix MatrixIdentity();
NFE_INLINE Matrix MatrixTranslation3(const Vector& pos);
NFCOMMON_API Matrix MatrixTranspose(const Matrix& M);
NFCOMMON_API Matrix MatrixInverse(const Matrix& M);
NFCOMMON_API Matrix MatrixRotationNormal(const Vector& NormalAxis, float Angle);
NFCOMMON_API Matrix MatrixPerspective(float aspect, float fovY, float farZ, float nearZ);
NFCOMMON_API Matrix MatrixOrtho(float left, float right, float bottom, float top, float zn,
                                float zf);
NFCOMMON_API Matrix MatrixLookTo(const Vector& EyePosition, const Vector& EyeDirection,
                                 const Vector& UpDirection);
NFCOMMON_API Matrix operator* (const Matrix& M1, const Matrix& M2);


NFE_INLINE Matrix MatrixIdentity()
{
    Matrix M;
    M.r[0] = g_IdentityR0;
    M.r[1] = g_IdentityR1;
    M.r[2] = g_IdentityR2;
    M.r[3] = g_IdentityR3;
    return M;
}

NFE_INLINE Matrix MatrixTranslation3(const Vector& pos)
{
    Matrix M;
    M.r[0] = g_IdentityR0;
    M.r[1] = g_IdentityR1;
    M.r[2] = g_IdentityR2;
    M.r[3] = Vector(pos.f[0], pos.f[1], pos.f[2], 1.0f);
    return M;
}

//multiplies 3D vector by matrix
NFE_INLINE Vector VectorTransform3(const Vector& v, const Matrix& M)
{
    __m128 vResult = _mm_shuffle_ps(v, v , _MM_SHUFFLE(0, 0, 0, 0));
    vResult = _mm_mul_ps(vResult, M.r[0]);
    __m128 vTemp = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
    vTemp = _mm_mul_ps(vTemp, M.r[1]);
    vResult = _mm_add_ps(vResult, vTemp);
    vTemp = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
    vTemp = _mm_mul_ps(vTemp, M.r[2]);
    vResult = _mm_add_ps(vResult, vTemp);
    vResult = _mm_add_ps(vResult, M.r[3]);

    return vResult;
}

//multiplies 4D vector by matrix
NFE_INLINE Vector VectorTransform4(const Vector& v, const Matrix& M)
{
    __m128 vResult, vTemp;

    vResult = _mm_shuffle_ps(v, v , _MM_SHUFFLE(0, 0, 0, 0));
    vResult = _mm_mul_ps(vResult, M.r[0]);

    vTemp = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
    vTemp = _mm_mul_ps(vTemp, M.r[1]);
    vResult = _mm_add_ps(vResult, vTemp);

    vTemp = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
    vTemp = _mm_mul_ps(vTemp, M.r[2]);
    vResult = _mm_add_ps(vResult, vTemp);

    vTemp = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
    vTemp = _mm_mul_ps(vTemp, M.r[3]);
    vResult = _mm_add_ps(vResult, vTemp);

    return vResult;
}

} // namespace Math
} // namespace NFE

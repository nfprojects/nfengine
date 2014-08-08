/**
    NFEngine project

    \file   Matrix.cpp
*/

#include "stdafx.h"
#include "Math.h"

namespace NFE {
namespace Math {

Matrix MatrixLookTo(const Vector& EyePosition, const Vector& EyeDirection,
                    const Vector& UpDirection)
{
    /*
    zaxis = normal(Eye - At)
    xaxis = normal(cross(Up, zaxis))
    yaxis = cross(zaxis, xaxis)

    xaxis.x           yaxis.x           zaxis.x          0
    xaxis.y           yaxis.y           zaxis.y          0
    xaxis.z           yaxis.z           zaxis.z          0
    dot(xaxis, eye)   dot(yaxis, eye)   dot(zaxis, eye)  1
    */

    Vector zaxis = VectorNormalize3(EyeDirection);
    Vector xaxis = VectorNormalize3(VectorCross3(UpDirection, zaxis));
    Vector yaxis = VectorCross3(zaxis, xaxis);

    Matrix M;

    M.r[0] = Vector(xaxis.f[0], yaxis.f[0], zaxis.f[0], 0.0f);
    M.r[1] = Vector(xaxis.f[1], yaxis.f[1], zaxis.f[1], 0.0f);
    M.r[2] = Vector(xaxis.f[2], yaxis.f[2], zaxis.f[2], 0.0f);
    M.r[3] = Vector(-VectorDot3(xaxis, EyePosition).f[0],
                    -VectorDot3(yaxis, EyePosition).f[0],
                    -VectorDot3(zaxis, EyePosition).f[0],
                    1.0f);
    return M;
}


Matrix MatrixPerspective(float aspect, float fovY, float farZ, float nearZ)
{
    Matrix M;
    float yScale = 1.0f / tanf(fovY * 0.5f);
    float xScale = yScale / aspect;

    M.r[0] = Vector(xScale, 0.0f,   0.0f,                   0.0f);
    M.r[1] = Vector(0.0f,       yScale, 0.0f,                   0.0f);
    M.r[2] = Vector(0.0f,       0.0f,   farZ / (farZ - nearZ),  1.0f);
    M.r[3] = Vector(0.0f,       0.0f,   -nearZ * farZ / (farZ - nearZ), 0.0f);
    return M;
}

Matrix MatrixOrtho(float left, float right, float bottom, float top, float zn, float zf)
{
    Matrix M;
    M.r[0] = Vector(2.0f / (right - left),          0.0f,                       0.0f,           0.0f);
    M.r[1] = Vector(0.0f,                       2.0f / (top - bottom),          0.0f,           0.0f);
    M.r[2] = Vector(0.0f,                       0.0f,                       1.0f / (zf - zn),   0.0f);
    M.r[3] = Vector((left + right) / (left - right), (top + bottom) / (bottom - top),   zn / (zn - zf),
                    1.0f);
    return M;
}

Matrix MatrixRotationNormal(const Vector& NormalAxis, float Angle)
{
    Vector               N0, N1;
    Vector               V0, V1, V2;
    Vector               R0, R1, R2;
    Vector               C0, C1, C2;
    Matrix               M;

    float fSinAngle = sinf(Angle);
    float fCosAngle = cosf(Angle);

    C2 = _mm_set_ps1(1.0f - fCosAngle);
    C1 = _mm_set_ps1(fCosAngle);
    C0 = _mm_set_ps1(fSinAngle);

    N0 = _mm_shuffle_ps(NormalAxis, NormalAxis, _MM_SHUFFLE(3, 0, 2, 1));
//    N0 = XMVectorPermute(NormalAxis, NormalAxis, SwizzleYZXW);
    N1 = _mm_shuffle_ps(NormalAxis, NormalAxis, _MM_SHUFFLE(3, 1, 0, 2));
//    N1 = XMVectorPermute(NormalAxis, NormalAxis, SwizzleZXYW);

    V0 = _mm_mul_ps(C2, N0);
    V0 = _mm_mul_ps(V0, N1);

    R0 = _mm_mul_ps(C2, NormalAxis);
    R0 = _mm_mul_ps(R0, NormalAxis);
    R0 = _mm_add_ps(R0, C1);

    R1 = _mm_mul_ps(C0, NormalAxis);
    R1 = _mm_add_ps(R1, V0);
    R2 = _mm_mul_ps(C0, NormalAxis);
    R2 = _mm_sub_ps(V0, R2);

    V0 = _mm_and_ps(R0, g_Mask3);
//    V0 = XMVectorSelect(A, R0, g_XMSelect1110);
    V1 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(2, 1, 2, 0));
    V1 = _mm_shuffle_ps(V1, V1, _MM_SHUFFLE(0, 3, 2, 1));
//    V1 = XMVectorPermute(R1, R2, Permute0Z1Y1Z0X);
    V2 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(0, 0, 1, 1));
    V2 = _mm_shuffle_ps(V2, V2, _MM_SHUFFLE(2, 0, 2, 0));
//    V2 = XMVectorPermute(R1, R2, Permute0Y1X0Y1X);

    R2 = _mm_shuffle_ps(V0, V1, _MM_SHUFFLE(1, 0, 3, 0));
    R2 = _mm_shuffle_ps(R2, R2, _MM_SHUFFLE(1, 3, 2, 0));
    M.r[0] = R2;
//    M.r[0] = XMVectorPermute(V0, V1, Permute0X1X1Y0W);
    R2 = _mm_shuffle_ps(V0, V1, _MM_SHUFFLE(3, 2, 3, 1));
    R2 = _mm_shuffle_ps(R2, R2, _MM_SHUFFLE(1, 3, 0, 2));
    M.r[1] = R2;
//    M.r[1] = XMVectorPermute(V0, V1, Permute1Z0Y1W0W);
    V2 = _mm_shuffle_ps(V2, V0, _MM_SHUFFLE(3, 2, 1, 0));
//    R2 = _mm_shuffle_ps(R2,R2,_MM_SHUFFLE(3,2,1,0));
    M.r[2] = V2;
//    M.r[2] = XMVectorPermute(V0, V2, Permute1X1Y0Z0W);
    M.r[3] = g_IdentityR3;
    return M;
}

Matrix MatrixTranspose(const Matrix& M)
{
    Vector vTemp1 = _mm_shuffle_ps(M.r[0], M.r[1], _MM_SHUFFLE(1, 0, 1, 0));
    Vector vTemp3 = _mm_shuffle_ps(M.r[0], M.r[1], _MM_SHUFFLE(3, 2, 3, 2));
    Vector vTemp2 = _mm_shuffle_ps(M.r[2], M.r[3], _MM_SHUFFLE(1, 0, 1, 0));
    Vector vTemp4 = _mm_shuffle_ps(M.r[2], M.r[3], _MM_SHUFFLE(3, 2, 3, 2));

    Matrix mResult;
    mResult.r[0] = _mm_shuffle_ps(vTemp1, vTemp2, _MM_SHUFFLE(2, 0, 2, 0));
    mResult.r[1] = _mm_shuffle_ps(vTemp1, vTemp2, _MM_SHUFFLE(3, 1, 3, 1));
    mResult.r[2] = _mm_shuffle_ps(vTemp3, vTemp4, _MM_SHUFFLE(2, 0, 2, 0));
    mResult.r[3] = _mm_shuffle_ps(vTemp3, vTemp4, _MM_SHUFFLE(3, 1, 3, 1));
    return mResult;
}

Matrix MatrixInverse(const Matrix& M)
{
    Matrix MT = MatrixTranspose(M);
    Vector V00 = _mm_shuffle_ps(MT.r[2], MT.r[2], _MM_SHUFFLE(1, 1, 0, 0));
    Vector V10 = _mm_shuffle_ps(MT.r[3], MT.r[3], _MM_SHUFFLE(3, 2, 3, 2));
    Vector V01 = _mm_shuffle_ps(MT.r[0], MT.r[0], _MM_SHUFFLE(1, 1, 0, 0));
    Vector V11 = _mm_shuffle_ps(MT.r[1], MT.r[1], _MM_SHUFFLE(3, 2, 3, 2));
    Vector V02 = _mm_shuffle_ps(MT.r[2], MT.r[0], _MM_SHUFFLE(2, 0, 2, 0));
    Vector V12 = _mm_shuffle_ps(MT.r[3], MT.r[1], _MM_SHUFFLE(3, 1, 3, 1));

    Vector D0 = _mm_mul_ps(V00, V10);
    Vector D1 = _mm_mul_ps(V01, V11);
    Vector D2 = _mm_mul_ps(V02, V12);

    V00 = _mm_shuffle_ps(MT.r[2], MT.r[2], _MM_SHUFFLE(3, 2, 3, 2));
    V10 = _mm_shuffle_ps(MT.r[3], MT.r[3], _MM_SHUFFLE(1, 1, 0, 0));
    V01 = _mm_shuffle_ps(MT.r[0], MT.r[0], _MM_SHUFFLE(3, 2, 3, 2));
    V11 = _mm_shuffle_ps(MT.r[1], MT.r[1], _MM_SHUFFLE(1, 1, 0, 0));
    V02 = _mm_shuffle_ps(MT.r[2], MT.r[0], _MM_SHUFFLE(3, 1, 3, 1));
    V12 = _mm_shuffle_ps(MT.r[3], MT.r[1], _MM_SHUFFLE(2, 0, 2, 0));

    V00 = _mm_mul_ps(V00, V10);
    V01 = _mm_mul_ps(V01, V11);
    V02 = _mm_mul_ps(V02, V12);
    D0 = _mm_sub_ps(D0, V00);
    D1 = _mm_sub_ps(D1, V01);
    D2 = _mm_sub_ps(D2, V02);
    // V11 = D0Y,D0W,D2Y,D2Y
    V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 1, 3, 1));
    V00 = _mm_shuffle_ps(MT.r[1], MT.r[1], _MM_SHUFFLE(1, 0, 2, 1));
    V10 = _mm_shuffle_ps(V11, D0, _MM_SHUFFLE(0, 3, 0, 2));
    V01 = _mm_shuffle_ps(MT.r[0], MT.r[0], _MM_SHUFFLE(0, 1, 0, 2));
    V11 = _mm_shuffle_ps(V11, D0, _MM_SHUFFLE(2, 1, 2, 1));
    // V13 = D1Y,D1W,D2W,D2W
    Vector V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 3, 3, 1));
    V02 = _mm_shuffle_ps(MT.r[3], MT.r[3], _MM_SHUFFLE(1, 0, 2, 1));
    V12 = _mm_shuffle_ps(V13, D1, _MM_SHUFFLE(0, 3, 0, 2));
    Vector V03 = _mm_shuffle_ps(MT.r[2], MT.r[2], _MM_SHUFFLE(0, 1, 0, 2));
    V13 = _mm_shuffle_ps(V13, D1, _MM_SHUFFLE(2, 1, 2, 1));

    Vector C0 = _mm_mul_ps(V00, V10);
    Vector C2 = _mm_mul_ps(V01, V11);
    Vector C4 = _mm_mul_ps(V02, V12);
    Vector C6 = _mm_mul_ps(V03, V13);

    // V11 = D0X,D0Y,D2X,D2X
    V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(0, 0, 1, 0));
    V00 = _mm_shuffle_ps(MT.r[1], MT.r[1], _MM_SHUFFLE(2, 1, 3, 2));
    V10 = _mm_shuffle_ps(D0, V11, _MM_SHUFFLE(2, 1, 0, 3));
    V01 = _mm_shuffle_ps(MT.r[0], MT.r[0], _MM_SHUFFLE(1, 3, 2, 3));
    V11 = _mm_shuffle_ps(D0, V11, _MM_SHUFFLE(0, 2, 1, 2));
    // V13 = D1X,D1Y,D2Z,D2Z
    V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(2, 2, 1, 0));
    V02 = _mm_shuffle_ps(MT.r[3], MT.r[3], _MM_SHUFFLE(2, 1, 3, 2));
    V12 = _mm_shuffle_ps(D1, V13, _MM_SHUFFLE(2, 1, 0, 3));
    V03 = _mm_shuffle_ps(MT.r[2], MT.r[2], _MM_SHUFFLE(1, 3, 2, 3));
    V13 = _mm_shuffle_ps(D1, V13, _MM_SHUFFLE(0, 2, 1, 2));

    V00 = _mm_mul_ps(V00, V10);
    V01 = _mm_mul_ps(V01, V11);
    V02 = _mm_mul_ps(V02, V12);
    V03 = _mm_mul_ps(V03, V13);
    C0 = _mm_sub_ps(C0, V00);
    C2 = _mm_sub_ps(C2, V01);
    C4 = _mm_sub_ps(C4, V02);
    C6 = _mm_sub_ps(C6, V03);

    V00 = _mm_shuffle_ps(MT.r[1], MT.r[1], _MM_SHUFFLE(0, 3, 0, 3));
    // V10 = D0Z,D0Z,D2X,D2Y
    V10 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 0, 2, 2));
    V10 = _mm_shuffle_ps(V10, V10, _MM_SHUFFLE(0, 2, 3, 0));
    V01 = _mm_shuffle_ps(MT.r[0], MT.r[0], _MM_SHUFFLE(2, 0, 3, 1));
    // V11 = D0X,D0W,D2X,D2Y
    V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 0, 3, 0));
    V11 = _mm_shuffle_ps(V11, V11, _MM_SHUFFLE(2, 1, 0, 3));
    V02 = _mm_shuffle_ps(MT.r[3], MT.r[3], _MM_SHUFFLE(0, 3, 0, 3));
    // V12 = D1Z,D1Z,D2Z,D2W
    V12 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 2, 2, 2));
    V12 = _mm_shuffle_ps(V12, V12, _MM_SHUFFLE(0, 2, 3, 0));
    V03 = _mm_shuffle_ps(MT.r[2], MT.r[2], _MM_SHUFFLE(2, 0, 3, 1));
    // V13 = D1X,D1W,D2Z,D2W
    V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 2, 3, 0));
    V13 = _mm_shuffle_ps(V13, V13, _MM_SHUFFLE(2, 1, 0, 3));

    V00 = _mm_mul_ps(V00, V10);
    V01 = _mm_mul_ps(V01, V11);
    V02 = _mm_mul_ps(V02, V12);
    V03 = _mm_mul_ps(V03, V13);
    Vector C1 = _mm_sub_ps(C0, V00);
    C0 = _mm_add_ps(C0, V00);
    Vector C3 = _mm_add_ps(C2, V01);
    C2 = _mm_sub_ps(C2, V01);
    Vector C5 = _mm_sub_ps(C4, V02);
    C4 = _mm_add_ps(C4, V02);
    Vector C7 = _mm_add_ps(C6, V03);
    C6 = _mm_sub_ps(C6, V03);

    C0 = _mm_shuffle_ps(C0, C1, _MM_SHUFFLE(3, 1, 2, 0));
    C2 = _mm_shuffle_ps(C2, C3, _MM_SHUFFLE(3, 1, 2, 0));
    C4 = _mm_shuffle_ps(C4, C5, _MM_SHUFFLE(3, 1, 2, 0));
    C6 = _mm_shuffle_ps(C6, C7, _MM_SHUFFLE(3, 1, 2, 0));
    C0 = _mm_shuffle_ps(C0, C0, _MM_SHUFFLE(3, 1, 2, 0));
    C2 = _mm_shuffle_ps(C2, C2, _MM_SHUFFLE(3, 1, 2, 0));
    C4 = _mm_shuffle_ps(C4, C4, _MM_SHUFFLE(3, 1, 2, 0));
    C6 = _mm_shuffle_ps(C6, C6, _MM_SHUFFLE(3, 1, 2, 0));

    // Get the determinate
    Vector vTemp = VectorDot4(C0, MT.r[0]);
    vTemp = _mm_div_ps(g_One, vTemp);

    Matrix mResult;
    mResult.r[0] = _mm_mul_ps(C0, vTemp);
    mResult.r[1] = _mm_mul_ps(C2, vTemp);
    mResult.r[2] = _mm_mul_ps(C4, vTemp);
    mResult.r[3] = _mm_mul_ps(C6, vTemp);
    return mResult;
}


Matrix operator* (const Matrix& M1, const Matrix& M2)
{
    Matrix mResult;
    // Cache the invariants in registers
    float x = M1.m[0][0];
    float y = M1.m[0][1];
    float z = M1.m[0][2];
    float w = M1.m[0][3];

    // Perform the operation on the first row
    mResult.m[0][0] = (M2.m[0][0] * x) + (M2.m[1][0] * y) + (M2.m[2][0] * z) + (M2.m[3][0] * w);
    mResult.m[0][1] = (M2.m[0][1] * x) + (M2.m[1][1] * y) + (M2.m[2][1] * z) + (M2.m[3][1] * w);
    mResult.m[0][2] = (M2.m[0][2] * x) + (M2.m[1][2] * y) + (M2.m[2][2] * z) + (M2.m[3][2] * w);
    mResult.m[0][3] = (M2.m[0][3] * x) + (M2.m[1][3] * y) + (M2.m[2][3] * z) + (M2.m[3][3] * w);

    // Repeat for all the other rows
    x = M1.m[1][0];
    y = M1.m[1][1];
    z = M1.m[1][2];
    w = M1.m[1][3];
    mResult.m[1][0] = (M2.m[0][0] * x) + (M2.m[1][0] * y) + (M2.m[2][0] * z) + (M2.m[3][0] * w);
    mResult.m[1][1] = (M2.m[0][1] * x) + (M2.m[1][1] * y) + (M2.m[2][1] * z) + (M2.m[3][1] * w);
    mResult.m[1][2] = (M2.m[0][2] * x) + (M2.m[1][2] * y) + (M2.m[2][2] * z) + (M2.m[3][2] * w);
    mResult.m[1][3] = (M2.m[0][3] * x) + (M2.m[1][3] * y) + (M2.m[2][3] * z) + (M2.m[3][3] * w);

    x = M1.m[2][0];
    y = M1.m[2][1];
    z = M1.m[2][2];
    w = M1.m[2][3];
    mResult.m[2][0] = (M2.m[0][0] * x) + (M2.m[1][0] * y) + (M2.m[2][0] * z) + (M2.m[3][0] * w);
    mResult.m[2][1] = (M2.m[0][1] * x) + (M2.m[1][1] * y) + (M2.m[2][1] * z) + (M2.m[3][1] * w);
    mResult.m[2][2] = (M2.m[0][2] * x) + (M2.m[1][2] * y) + (M2.m[2][2] * z) + (M2.m[3][2] * w);
    mResult.m[2][3] = (M2.m[0][3] * x) + (M2.m[1][3] * y) + (M2.m[2][3] * z) + (M2.m[3][3] * w);

    x = M1.m[3][0];
    y = M1.m[3][1];
    z = M1.m[3][2];
    w = M1.m[3][3];
    mResult.m[3][0] = (M2.m[0][0] * x) + (M2.m[1][0] * y) + (M2.m[2][0] * z) + (M2.m[3][0] * w);
    mResult.m[3][1] = (M2.m[0][1] * x) + (M2.m[1][1] * y) + (M2.m[2][1] * z) + (M2.m[3][1] * w);
    mResult.m[3][2] = (M2.m[0][2] * x) + (M2.m[1][2] * y) + (M2.m[2][2] * z) + (M2.m[3][2] * w);
    mResult.m[3][3] = (M2.m[0][3] * x) + (M2.m[1][3] * y) + (M2.m[2][3] * z) + (M2.m[3][3] * w);
    return mResult;
}

} // namespace Math
} // namespace NFE

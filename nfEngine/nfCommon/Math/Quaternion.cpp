/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Quaternion function definitions.
 */

#include "PCH.hpp"
#include "Quaternion.hpp"

namespace NFE {
namespace Math {

// Create rotation matrix from quaternion. Quaternion must be normalized!
Matrix MatrixFromQuaternion(const Quaternion& q)
{
    float xx = q.f[0] * q.f[0], yy = q.f[1] * q.f[1], zz = q.f[2] * q.f[2];
    float xy = q.f[0] * q.f[1], xz = q.f[0] * q.f[2];
    float yz = q.f[1] * q.f[2], wx = q.f[3] * q.f[0];
    float wy = q.f[3] * q.f[1], wz = q.f[3] * q.f[2];

    Matrix m;
    m.m[0][0] = 1.0f - 2.0f * ( yy + zz );
    m.m[0][1] = 2.0f * ( xy + wz );
    m.m[0][2] = 2.0f * ( xz - wy );
    m.m[0][3] = 0.0f;

    m.m[1][0] = 2.0f * ( xy - wz );
    m.m[1][1] = 1.0f - 2.0f * ( xx + zz );
    m.m[1][2] = 2.0f * ( yz + wx );
    m.m[1][3] = 0.0f;

    m.m[2][0] = 2.0f * ( xz + wy );
    m.m[2][1] = 2.0f * ( yz - wx );
    m.m[2][2] = 1.0f - 2.0f * ( xx + yy );
    m.m[2][3] = 0.0f;

    m.m[3][0] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[3][2] = 0.0f;
    m.m[3][3] = 1.0f;

    return m;
}

// Create quaternion from 4x4 matrix (only 3x3 is considered)
Quaternion QuaternionFromMatrix(const Matrix& m)
{
    Quaternion q;
    q.f[0] = sqrtf(Max(0.0f, 1.0f + m.m[0][0] - m.m[1][1] - m.m[2][2])) / 2.0f;
    q.f[1] = sqrtf(Max(0.0f, 1.0f - m.m[0][0] + m.m[1][1] - m.m[2][2])) / 2.0f;
    q.f[2] = sqrtf(Max(0.0f, 1.0f - m.m[0][0] - m.m[1][1] + m.m[2][2])) / 2.0f;
    q.f[3] = sqrtf(Max(0.0f, 1.0f + m.m[0][0] + m.m[1][1] + m.m[2][2])) / 2.0f;

    q.f[0] = CopySignF(q.f[0], m.m[1][2] - m.m[2][1]);
    q.f[1] = CopySignF(q.f[1], m.m[2][0] - m.m[0][2]);
    q.f[2] = CopySignF(q.f[2], m.m[0][1] - m.m[1][0]);
    return q;
}

// Spherical interpolation of two quaternions
Quaternion QuaternionInterpolate(const Quaternion& q0, const Quaternion& q1, float t)
{
    float cosOmega = VectorDot4(q0, q1).f[0];
    Quaternion new_q1 = q1;
    if (cosOmega < 0.0f)
    {
        new_q1 = -new_q1;
        cosOmega = -cosOmega;
    }

    float k0, k1;
    if (cosOmega > 0.9999f)
    {
        k0 = 1.0f - t;
        k1 = t;
    }
    else
    {
        float sinOmega = sqrtf(1.0f - cosOmega * cosOmega);
        float omega = atan2f(sinOmega, cosOmega);
        float oneOverSinOmega = 1.0f / sinOmega;
        k0 = sinf((1.0f - t) * omega) * oneOverSinOmega;
        k1 = sinf(t * omega) * oneOverSinOmega;
    }

    return q0 * k0 + new_q1 * k1;
}

} // namespace Math
} // namespace NFE

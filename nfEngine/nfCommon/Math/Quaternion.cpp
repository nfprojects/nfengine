/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Quaternion function definitions.
 */

#include "../PCH.hpp"
#include "Quaternion.hpp"

namespace NFE {
namespace Math {

Quaternion QuaternionFromAngles(float pitch, float yaw, float roll)
{
    // based on: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    pitch *= 0.5f;
    yaw *= 0.5f;
    roll *= 0.5f;

    Quaternion q;
    float t0 = cosf(yaw);
    float t1 = sinf(yaw);
    float t2 = cosf(roll);
    float t3 = sinf(roll);
    float t4 = cosf(pitch);
    float t5 = sinf(pitch);

    const Vector term0 = Vector(t0, t0, t1, t0);
    const Vector term1 = Vector(t3, t2, t2, t2);
    const Vector term2 = Vector(t4, t5, t4, t4);

    const Vector term3 = Vector(-t1, t1, -t0, t1);
    const Vector term4 = Vector(t2, t3, t3, t3);
    const Vector term5 = Vector(t5, t4, t5, t5);
 
    return term0 * term1 * term2 + term3 * term4 * term5;
}

Vector QuaternionRotateVector(const Quaternion& q, const Vector& v)
{
    // based on: http://gamedev.stackexchange.com/a/50545

    // TODO write SSE version

    // extract the scalar part
    const float s = q[3];

    // extract vector part
    const Vector u = Vector(q[0], q[1], q[2], 0.0f);

    return 2.0f * VectorDot3(u, v) * u + (s * s - VectorDot3f(u, u)) * v + (2.0f * s) * VectorCross3(u, v);
}

Vector QuaternionToAxis(const Quaternion& q)
{
    float x = q.f[0];
    float y = q.f[1];
    float z = q.f[2];
    float sin_squared = x * x + y * y + z * z;

    Vector result;
    if (sin_squared > 0.0f)
    {
        float sin_theta = sqrtf(sin_squared);
        float k = 2.0f * atan2f(sin_theta, q.f[3]) / sin_theta;
        result = q * k;
    }
    else
    {
        result = 2.0f * q;
    }

    result.f[3] = 0.0f;
    return result;
}

// Create rotation matrix from quaternion. Quaternion must be normalized!
Matrix QuaternionToMatrix(const Quaternion& q)
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

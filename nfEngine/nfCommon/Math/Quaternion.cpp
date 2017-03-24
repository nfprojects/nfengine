/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Quaternion function definitions.
 */

#include "PCH.hpp"
#include "Quaternion.hpp"

namespace NFE {
namespace Math {

Quaternion Quaternion::FromAxis(const Vector& axis, float angle)
{
    angle *= 0.5f;
    Quaternion q = axis * sinf(angle);
    q.f[3] = cosf(angle);
    return q;
}

Quaternion Quaternion::RotationX(float angle)
{
    angle *= 0.5f;
    return Quaternion(sinf(angle), 0.0f, 0.0f, cosf(angle));
}

Quaternion Quaternion::RotationY(float angle)
{
    angle *= 0.5f;
    return Quaternion(0.0f, sinf(angle), 0.0f, cosf(angle));
}

Quaternion Quaternion::RotationZ(float angle)
{
    angle *= 0.5f;
    return Quaternion(0.0f, 0.0f, sinf(angle), cosf(angle));
}

Quaternion Quaternion::operator * (const Quaternion& q2) const
{
    // TODO: convert to SSE!

    Quaternion result;
    result.f[0] = f[3] * q2.f[0] + f[0] * q2.f[3] + f[1] * q2.f[2] - f[2] * q2.f[1];
    result.f[1] = f[3] * q2.f[1] + f[1] * q2.f[3] + f[2] * q2.f[0] - f[0] * q2.f[2];
    result.f[2] = f[3] * q2.f[2] + f[2] * q2.f[3] + f[0] * q2.f[1] - f[1] * q2.f[0];
    result.f[3] = f[3] * q2.f[3] - f[0] * q2.f[0] - f[1] * q2.f[1] - f[2] * q2.f[2];
    return result;
}

Quaternion Quaternion::Inverted() const
{
    Quaternion result;
    result.q.f[3] = q.f[3];
    result.q.f[0] = -q.f[0];
    result.q.f[1] = -q.f[1];
    result.q.f[2] = -q.f[2];
    result.q.Normalize4();
    return result;
}

Quaternion& Quaternion::Invert()
{
    q.f[3] = q.f[3];
    q.f[0] = -q.f[0];
    q.f[1] = -q.f[1];
    q.f[2] = -q.f[2];
    q.Normalize4();

    return *this;
}

Quaternion Quaternion::FromAngles(float yaw, float pitch, float roll)
{
    // based on: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    pitch *= 0.5f;
    yaw *= 0.5f;
    roll *= 0.5f;

    Quaternion q;
    float t0 = cosf(roll);
    float t1 = sinf(roll);
    float t2 = cosf(pitch);
    float t3 = sinf(pitch);
    float t4 = cosf(yaw);
    float t5 = sinf(yaw);

    const Vector term0 = Vector(t0, t0, t1, t0);
    const Vector term1 = Vector(t3, t2, t2, t2);
    const Vector term2 = Vector(t4, t5, t4, t4);

    const Vector term3 = Vector(-t1, t1, -t0, t1);
    const Vector term4 = Vector(t2, t3, t3, t3);
    const Vector term5 = Vector(t5, t4, t5, t5);
 
    return term0 * term1 * term2 + term3 * term4 * term5;
}

Quaternion Quaternion::FromMatrix(const Matrix& m)
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

Vector Quaternion::Transform(const Vector& v) const
{
    // based on: http://gamedev.stackexchange.com/a/50545

    // TODO write SSE version

    // extract the scalar part
    const float s = q[3];

    // extract vector part
    const Vector u = Vector(q[0], q[1], q[2], 0.0f);

    return 2.0f * Vector::Dot3V(u, v) * u + (s * s - Vector::Dot3(u, u)) * v + (2.0f * s) * Vector::Cross3(u, v);
}

Vector Quaternion::ToAxis() const
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

Matrix Quaternion::ToMatrix() const
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

void Quaternion::ToAngles(float& outYaw, float& outPitch, float& outRoll) const
{
    // based on: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    const float t0 = 2.0f * (q[0] * q[1] + q[2] * q[3]);
    const float t1 = 2.0f * (q[0] * q[2] - q[3] * q[1]);
    const float t2 = 2.0f * (q[0] * q[3] - q[1] * q[2]);
    const float t3 = 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]);
    const float t4 = 1.0f - 2.0f * (q[2] * q[2] + q[3] * q[3]);

    outRoll = atan2f(t0, t3);
    outPitch = asinf(t1);
    outYaw = atan2f(t2, t4);
}

Quaternion Quaternion::Interpolate(const Quaternion& q0, const Quaternion& q1, float t)
{
    float cosOmega = Vector::Dot4(q0, q1);
    Vector new_q1 = q1;
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

    return q0.q * k0 + new_q1 * k1;
}

} // namespace Math
} // namespace NFE

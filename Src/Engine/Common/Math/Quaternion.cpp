#include "PCH.hpp"
#include "Quaternion.hpp"
#include "Transcendental.hpp"

namespace NFE {
namespace Math {

bool Quaternion::IsValid() const
{
    if (!q.IsValid())
    {
        return false;
    }

    // check if normalized
    return Abs(q.SqrLength4() - 1.0f) < 0.001f;
}

const Quaternion Quaternion::FromAxisAndAngle(const Vec4f& axis, float angle)
{
    angle *= 0.5f;
    Quaternion q = Quaternion(axis * Sin(angle));
    q.q.w = Cos(angle);
    return q;
}

const Quaternion Quaternion::RotationX(float angle)
{
    angle *= 0.5f;
    return Quaternion(Sin(angle), 0.0f, 0.0f, Cos(angle));
}

const Quaternion Quaternion::RotationY(float angle)
{
    angle *= 0.5f;
    return Quaternion(0.0f, Sin(angle), 0.0f, Cos(angle));
}

const Quaternion Quaternion::RotationZ(float angle)
{
    angle *= 0.5f;
    return Quaternion(0.0f, 0.0f, Sin(angle), Cos(angle));
}

const Quaternion Quaternion::operator * (const Quaternion& b) const
{
    const Vec4f a0120 = q.Swizzle<0, 1, 2, 0>();
    const Vec4f b3330 = b.q.Swizzle<3, 3, 3, 0>();
    const Vec4f t1 = a0120 * b3330;

    const Vec4f a1201 = q.Swizzle<1, 2, 0, 1>();
    const Vec4f b2011 = b.q.Swizzle<2, 0, 1, 1>();
    const Vec4f t12 = Vec4f::MulAndAdd(a1201, b2011, t1);

    const Vec4f a3333 = q.Swizzle<3, 3, 3, 3>();
    const Vec4f b0123 = b.q;
    const Vec4f t0 = a3333 * b0123;

    const Vec4f a2012 = q.Swizzle<2, 0, 1, 2>();
    const Vec4f b1202 = b.q.Swizzle<1, 2, 0, 2>();
    const Vec4f t03 = Vec4f::NegMulAndAdd(a2012, b1202, t0);

    return Quaternion(t03 + t12.ChangeSign<false, false, false, true>());
}

const Quaternion Quaternion::Inverted() const
{
    Quaternion result = Conjugate();
    result.q.Normalize4();
    return result;
}

Quaternion& Quaternion::Invert()
{
    *this = Conjugate();
    q.Normalize4();

    return *this;
}

const Vec4f Quaternion::TransformVector(const Vec4f& v) const
{
    // based on identity:
    //
    // t = 2 * cross(q.xyz, v)
    // v' = v + q.w * t + cross(q.xyz, t)

    Vec4f t = Vec4f::Cross3(q, v);
    t = t + t;
    return Vec4f::MulAndAdd(t, q.w, v) + Vec4f::Cross3(q, t);
}

const Vec3x8f Quaternion::TransformVector(const Vec3x8f& v) const
{
    const Vec3x8f q8(q);
    Vec3x8f t = Vec3x8f::Cross(q8, v);
    t = t + t;
    return Vec3x8f::MulAndAdd(t, Vec8f(q.w), v) + Vec3x8f::Cross(q8, t);
}

void Quaternion::ToAxis(Vec4f& outAxis, float& outAngle) const
{
    Quaternion normalized = *this;
    if (normalized.q[3] > 1.0f)
    {
        normalized = Normalized();
    }

    const float scalar = normalized.q[3];
    outAngle = 2.0f * acosf(scalar);

    const float s = Sqrt(1.0f - scalar * scalar);
    outAxis = normalized.q;
    if (s >= 0.001f)
    {
        outAxis /= s;
    }

    outAxis.w = 0.0f;
}

const Quaternion Quaternion::Interpolate(const Quaternion& q0, const Quaternion& q1, float t)
{
    float cosOmega = Vec4f::Dot4(q0, q1);
    Vec4f new_q1 = q1;
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
        float sinOmega = Sqrt(1.0f - cosOmega * cosOmega);
        float omega = atan2f(sinOmega, cosOmega);
        float oneOverSinOmega = 1.0f / sinOmega;
        k0 = Sin((1.0f - t) * omega) * oneOverSinOmega;
        k1 = Sin(t * omega) * oneOverSinOmega;
    }

    return Quaternion(q0.q * k0 + new_q1 * k1);
}

bool Quaternion::AlmostEqual(const Quaternion& a, const Quaternion& b, float epsilon)
{
    float d = Vec4f::Dot4(a.q, b.q);
    return Abs(d) > 1.0f - epsilon;
}

const Quaternion Quaternion::FromEulerAngles(const Vec3f& angles)
{
    // based on: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    const float pitch = angles.x * 0.5f;
    const float yaw = angles.y * 0.5f;
    const float roll = angles.z * 0.5f;

    Quaternion q;
    float t0 = Cos(yaw);
    float t1 = Sin(yaw);
    float t2 = Cos(roll);
    float t3 = Sin(roll);
    float t4 = Cos(pitch);
    float t5 = Sin(pitch);

    const Vec4f term0 = Vec4f(t0, t1, t0, t0);
    const Vec4f term1 = Vec4f(t2, t2, t3, t2);
    const Vec4f term2 = Vec4f(t5, t4, t4, t4);

    const Vec4f term3 = Vec4f(t1, -t0, -t1, t1);
    const Vec4f term4 = Vec4f(t3, t3, t2, t3);
    const Vec4f term5 = Vec4f(t4, t5, t5, t5);

    return Quaternion(term0 * term1 * term2 + term3 * term4 * term5);
}

const Quaternion Quaternion::FromMatrix(const Matrix4& m)
{
    const Vec4f x = Vec4f(m.m[0][0]).ChangeSign<false, true, true, false>();
    const Vec4f y = Vec4f(m.m[1][1]).ChangeSign<true, false, true, false>();
    const Vec4f z = Vec4f(m.m[2][2]).ChangeSign<true, true, false, false>();

    Quaternion q;
    q.q = (Vec4f(1.0f) + x) + (y + z);
    q.q = Vec4f::Max(q.q, Vec4f::Zero());
    q.q = Vec4f::Sqrt(q.q) * 0.5f;

    q.q.x = CopySign(q.q.x, m.m[1][2] - m.m[2][1]);
    q.q.y = CopySign(q.q.y, m.m[2][0] - m.m[0][2]);
    q.q.z = CopySign(q.q.z, m.m[0][1] - m.m[1][0]);
    return q;
}

const Matrix4 Quaternion::ToMatrix() const
{
    Matrix4 m;
    m.rows[0] = GetAxisX();
    m.rows[1] = GetAxisY();
    m.rows[2] = GetAxisZ();
    m.rows[3] = VECTOR_W;
    return m;
}

const Vec3f Quaternion::ToEulerAngles() const
{
    // based on: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    float t0 = 2.0f * (q[3] * q[2] + q[0] * q[1]);
    float t2 = 2.0f * (q[3] * q[0] - q[1] * q[2]);
    float t3 = 2.0f * (q[3] * q[1] + q[2] * q[0]);
    float t1 = 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
    float t4 = 1.0f - 2.0f * (q[0] * q[0] + q[1] * q[1]);

    t2 = Clamp(t2, -1.0f, 1.0f);

    float pitch = asinf(t2);
    float yaw = atan2f(t3, t4);
    float roll = atan2f(t0, t1);

    if (pitch > NFE_MATH_PI)
    {
        pitch -= 2.0f * NFE_MATH_PI;
    }
    if (pitch < -NFE_MATH_PI)
    {
        pitch += 2.0f * NFE_MATH_PI;
    }

    if (yaw > NFE_MATH_PI)
    {
        yaw -= 2.0f * NFE_MATH_PI;
    }
    if (yaw < -NFE_MATH_PI)
    {
        yaw += 2.0f * NFE_MATH_PI;
    }

    if (roll > NFE_MATH_PI)
    {
        roll -= 2.0f * NFE_MATH_PI;
    }
    if (roll < -NFE_MATH_PI)
    {
        roll += 2.0f * NFE_MATH_PI;
    }

    return Vec3f{ pitch, yaw, roll };
}

} // namespace Math
} // namespace NFE

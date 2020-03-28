#pragma once

namespace NFE {
namespace Math {

const Quaternion Quaternion::Identity()
{
    return Quaternion(Vec4f(0.0f, 0.0f, 0.0f, 1.0f));
}

const Quaternion Quaternion::Conjugate() const
{
    Quaternion result;
    result.q = q.ChangeSign<true, true, true, false>();
    return result;
}

const Quaternion Quaternion::Normalized() const
{
    return Quaternion(q.Normalized4());
}

Quaternion& Quaternion::Normalize()
{
    q.Normalize4();
    return *this;
}

Quaternion& Quaternion::operator *= (const Quaternion& q2)
{
    *this = *this * q2;
    return *this;
}

const Vec4f Quaternion::GetAxisX() const
{
    return Vec4f(
        1.0f - 2.0f * (q.y * q.y + q.z * q.z),
        2.0f * (q.x * q.y + q.w * q.z),
        2.0f * (q.x * q.z - q.w * q.y),
        0.0f
    );
}

const Vec4f Quaternion::GetAxisY() const
{
    return Vec4f(
        2.0f * (q.x * q.y - q.w * q.z),
        1.0f - 2.0f * (q.x * q.x + q.z * q.z),
        2.0f * (q.y * q.z + q.w * q.x),
        0.0f
    );
}

const Vec4f Quaternion::GetAxisZ() const
{
    return Vec4f(
        2.0f * (q.x * q.z + q.w * q.y),
        2.0f * (q.y * q.z - q.w * q.x),
        1.0f - 2.0f * (q.x * q.x + q.y * q.y),
        0.0f
    );
}


} // namespace Math
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Quaternion class implementation.
 */

#pragma once

namespace NFE {
namespace Math {

Quaternion Quaternion::Identity()
{
    return Quaternion(Vector(0.0f, 0.0f, 0.0f, 1.0f));
}

Quaternion Quaternion::Conjugate() const
{
    Quaternion result;
    result.q = q.ChangeSign<true, true, true, false>();
    return result;
}

Quaternion Quaternion::Normalized() const
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

Vector Quaternion::GetAxisX() const
{
    return Vector(
        1.0f - 2.0f * (q.y * q.y + q.z * q.z),
        2.0f * (q.x * q.y + q.w * q.z),
        2.0f * (q.x * q.z - q.w * q.y)
    );
}

Vector Quaternion::GetAxisY() const
{
    return Vector(
        2.0f * (q.x * q.y - q.w * q.z),
        1.0f - 2.0f * (q.x * q.x + q.z * q.z),
        2.0f * (q.y * q.z + q.w * q.x)
    );
}

Vector Quaternion::GetAxisZ() const
{
    return Vector(
        2.0f * (q.x * q.z + q.w * q.y),
        2.0f * (q.y * q.z - q.w * q.x),
        1.0f - 2.0f * (q.x * q.x + q.y * q.y)
    );
}

} // namespace Math
} // namespace NFE

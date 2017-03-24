/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Quaternion class implementation (FPU version).
 */

#pragma once

namespace NFE {
namespace Math {

Quaternion Quaternion::Identity()
{
    return Vector(0.0f, 0.0f, 0.0f, 1.0f);
}

Quaternion Quaternion::Normalized() const
{
    return q.Normalized4();
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

} // namespace Math
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Quaternion class declarations.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"


namespace NFE {
namespace Math {

/**
 * Create null rotation quaternion.
 */
NFE_INLINE Quaternion QuaternionIdentity()
{
    return Vector(0.0f, 0.0f, 0.0f, 1.0f);
}

/**
 * Create quaternion form axis and angle.
 */
NFE_INLINE Quaternion QuaternionFromAxis(const Vector& axis, float angle)
{
    angle *= 0.5f;
    Quaternion q = axis * sinf(angle);
    q.f[3] = cosf(angle);
    return q;
}


NFE_INLINE float safe_acos(float x)
{
    if (x <= -1.0f) return NFE_MATH_PI;
    if (x >= 1.0f) return 0.0f;
    return acosf(x);
}

/**
 * Extract rotation axis from a quaternion.
 */
NFE_INLINE Vector QuaternionToAxis(const Quaternion& quat)
{
    float x = quat.f[0];
    float y = quat.f[1];
    float z = quat.f[2];
    float sin_squared = x * x + y * y + z * z;

    Vector result;
    if (sin_squared > 0.0f)
    {
        float sin_theta = sqrtf(sin_squared);
        float k = 2.0f * atan2f(sin_theta, quat.f[3]) / sin_theta;
        result = quat * k;
    }
    else
    {
        result = 2.0f * quat;
    }
    result.f[3] = 0.0f;
    return result;
}

/**
 * Create quaternion representing rotation around X axis.
 */
NFE_INLINE Quaternion QuaternionRotationX(float angle)
{
    angle *= 0.5f;
    Quaternion q = Quaternion(sinf(angle), 0.0f, 0.0f, cosf(angle));
    return q;
}

/**
 * Create quaternion representing rotation around Y axis.
 */
NFE_INLINE Quaternion QuaternionRotationY(float angle)
{
    angle *= 0.5f;
    Quaternion q = Quaternion(0.0f, sinf(angle), 0.0f, cosf(angle));
    return q;
}

/**
 * Create quaternion representing rotation around Z axis.
 */
NFE_INLINE Quaternion QuaternionRotationZ(float angle)
{
    angle *= 0.5f;
    Quaternion q = Quaternion(0.0f, 0.0f, sinf(angle), cosf(angle));
    return q;
}

/**
 * Multiply two quaternions.
 */
NFE_INLINE Quaternion QuaternionMultiply(const Quaternion& q1, const Quaternion& q2)
{
    // TODO: convert to SSE!

    Quaternion q;
    q.f[0] = q1.f[3] * q2.f[0] + q1.f[0] * q2.f[3] + q1.f[1] * q2.f[2] - q1.f[2] * q2.f[1];
    q.f[1] = q1.f[3] * q2.f[1] + q1.f[1] * q2.f[3] + q1.f[2] * q2.f[0] - q1.f[0] * q2.f[2];
    q.f[2] = q1.f[3] * q2.f[2] + q1.f[2] * q2.f[3] + q1.f[0] * q2.f[1] - q1.f[1] * q2.f[0];
    q.f[3] = q1.f[3] * q2.f[3] - q1.f[0] * q2.f[0] - q1.f[1] * q2.f[1] - q1.f[2] * q2.f[2];
    return q;
}

NFE_INLINE Quaternion QuaternionNormalize(const Quaternion& q)
{
    return VectorNormalize4(q);
}

/**
 * Calculate inverse of quaternion.
 */
NFE_INLINE Quaternion QuaternionInverse(const Quaternion& q)
{
    Quaternion result;
    result.f[3] =   q.f[3];
    result.f[0] = - q.f[0];
    result.f[1] = - q.f[1];
    result.f[2] = - q.f[2];
    return result / VectorLength4(q);
}

/**
 * Spherical interpolation of two quaternions.
 * @param q0,q1 Quaternions to interpolate.
 * @param t     Interpolation factor.
 * @return Interpolated quaternion (equal to q0 when t=0.0f and equal tp q1 when t=1.0f).
 */
NFCOMMON_API Quaternion QuaternionInterpolate(const Quaternion& q0, const Quaternion& q1, float t);

/**
 * Create rotation matrix from quaternion.
 * @note Quaternion must be normalized.
 * @return Matrix representing the same rotation as input quaternion.
 */
NFCOMMON_API Matrix MatrixFromQuaternion(const Quaternion& q);

/**
 * Create quaternion from 4x4 matrix.
 * @note Only 3x3 is considered.
 * @return Quaternion representing the same rotation as input matrix.
 */
NFCOMMON_API Quaternion QuaternionFromMatrix(const Matrix& m);

} // namespace Math
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Quaternion class declarations.
 */

#pragma once

#include "Math.hpp"
#include "Vector4.hpp"
#include "Matrix4.hpp"


namespace NFE {
namespace Math {

/**
 * Class representing 4D quaternion. Useful for representing rotations.
 */
struct NFE_ALIGN(16) Quaternion final
{
    // XYZ - vector part, W - scalar part:
    // q = f[3] + i * q[0] + j * q[1] + k * q[2]
    union
    {
        Vector4 q;
        float f[4];
    };

    Quaternion() : q(0.0f, 0.0f, 0.0f, 1.0f) { }
    Quaternion(const Quaternion&) = default;
    explicit Quaternion(const Vector4& v) : q(v) { }
    explicit Quaternion(float i, float j, float k, float s) : q(i, j, k, s) { }

    operator const Vector4&() const { return q; }
    operator Vector4&() { return q; }

    /**
     * Get transformed X, Y, Z axes.
     */
    NFE_INLINE Vector4 GetAxisX() const;
    NFE_INLINE Vector4 GetAxisY() const;
    NFE_INLINE Vector4 GetAxisZ() const;

    /**
     * Create null rotation quaternion.
     */
    NFE_INLINE static Quaternion Identity();

    /**
     * Create quaternion form axis and angle.
     * @note    Returned quaternion is be normalized.
     */
    NFCOMMON_API static Quaternion FromAxisAndAngle(const Vector4& axis, float angle);

    /**
     * Create quaternion representing rotation around X axis.
     * @note    Returned quaternion is be normalized.
     */
    NFCOMMON_API static Quaternion RotationX(float angle);

    /**
     * Create quaternion representing rotation around Y axis.
     * @note    Returned quaternion is be normalized.
     */
    NFCOMMON_API static Quaternion RotationY(float angle);

    /**
     * Create quaternion representing rotation around Z axis.
     * @note    Returned quaternion is be normalized.
     */
    NFCOMMON_API static Quaternion RotationZ(float angle);

    /**
     * Create quaternion from 4x4 matrix.
     * @note Only 3x3 is considered.
     * @return Quaternion representing the same rotation as input matrix.
     */
    NFCOMMON_API static Quaternion FromMatrix(const Matrix4& m);

    /**
     * Create quaternion from Euler angles.
     * @param   pitch   Rotation in X axis (in radians).
     * @param   yaw     Rotation in Y axis (in radians).
     * @param   roll    Rotation in Z axis (in radians).
     */
    NFCOMMON_API static Quaternion FromAngles(float pitch, float yaw, float roll);

    /**
     * Quaternion multiplication.
     */
    NFCOMMON_API Quaternion operator * (const Quaternion& q2) const;
    NFE_INLINE Quaternion& operator *= (const Quaternion& q2);

    /**
     * Turn to unit quaternion (length = 1.0f).
     */
    NFE_INLINE Quaternion& Normalize();
    NFE_INLINE Quaternion Normalized() const;

    /**
     * Return conjugate of quaternion
     * Equals to Inverted() if quaternion is normalized.
     */
    NFE_INLINE Quaternion Conjugate() const;

    /**
     * Calculate inverse of quaternion.
     */
    NFCOMMON_API Quaternion Inverted() const;

    /**
     * Invert this quaternion.
     */
    NFCOMMON_API Quaternion& Invert();

    /**
     * Rotate a 3D vector with a quaternion.
     */
    NFCOMMON_API Vector4 TransformVector(const Vector4& v) const;

    /**
     * Extract rotation axis and angle from a quaternion.
     * @param   outAxis     Normalized rotation axis.
     * @param   outAngle    Rotation angle in radians.
     * @note    This is slow.
     */
    NFCOMMON_API void ToAxis(Vector4& outAxis, float& outAngle) const;

    /**
     * Create rotation matrix from quaternion.
     * @note Quaternion must be normalized.
     * @return Matrix representing the same rotation as input quaternion.
     */
    NFCOMMON_API Matrix4 ToMatrix() const;

    /**
     * Convert quaternion to Euler angles.
     * @param   outPitch   Returned rotation in X axis (in radians).
     * @param   outYaw     Returned rotation in Y axis (in radians).
     * @param   outRoll    Returned rotation in Z axis (in radians).
     * @note    This is quite costly method.
     */
    NFCOMMON_API void ToAngles(float& outPitch, float& outYaw, float& outRoll) const;

    /**
     * Spherical interpolation of two quaternions.
     * @param q0,q1 Quaternions to interpolate.
     * @param t     Interpolation factor.
     * @return Interpolated quaternion (equal to q0 when t=0.0f and equal to q1 when t=1.0f).
     */
    NFCOMMON_API static Quaternion Interpolate(const Quaternion& q0, const Quaternion& q1, float t);

    /**
     * Check if two quaternions are almost equal.
     */
    NFCOMMON_API static bool AlmostEqual(const Quaternion& a, const Quaternion& b, float epsilon = NFE_MATH_EPSILON);
};

} // namespace Math
} // namespace NFE


#include "QuaternionImpl.hpp"

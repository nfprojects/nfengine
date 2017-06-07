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
 * Class representing 4D quaternion. Useful for representing rotations.
 */
struct NFCOMMON_API NFE_ALIGN(16) Quaternion final
{
    // XYZ - vector part, W - scalar part:
    // q = f[3] + i * q[0] + j * q[1] + k * q[2]
    union
    {
        Vector q;
        float f[4];
    };

    NFE_INLINE Quaternion()
        : q(0.0f, 0.0f, 0.0f, 1.0f)
    {}

    Quaternion(const Quaternion&) = default;

    NFE_INLINE Quaternion(const Vector& v)
        : q(v)
    { }

    NFE_INLINE Quaternion(float i, float j, float k, float s)
        : q(i, j, k, s)
    {}

    NFE_INLINE operator const Vector&() const { return q; }
    NFE_INLINE operator Vector&() { return q; }

    /**
     * Create null rotation quaternion.
     */
    NFE_INLINE static Quaternion Identity();

    /**
     * Create quaternion form axis and angle.
     * @note    Returned quaternion is be normalized.
     */
    static Quaternion FromAxisAndAngle(const Vector& axis, float angle);

    /**
     * Create quaternion representing rotation around X axis.
     * @note    Returned quaternion is be normalized.
     */
    static Quaternion RotationX(float angle);

    /**
     * Create quaternion representing rotation around Y axis.
     * @note    Returned quaternion is be normalized.
     */
    static Quaternion RotationY(float angle);

    /**
     * Create quaternion representing rotation around Z axis.
     * @note    Returned quaternion is be normalized.
     */
    static Quaternion RotationZ(float angle);

    /**
     * Create quaternion from 4x4 matrix.
     * @note Only 3x3 is considered.
     * @return Quaternion representing the same rotation as input matrix.
     */
    static Quaternion FromMatrix(const Matrix& m);

    /**
     * Create quaternion from Euler angles.
     * @param   pitch   Rotation in X axis (in radians).
     * @param   yaw     Rotation in Y axis (in radians).
     * @param   roll    Rotation in Z axis (in radians).
     */
    static Quaternion FromAngles(float pitch, float yaw, float roll);

    /**
     * Quaternion multiplication.
     */
    Quaternion operator * (const Quaternion& q2) const;
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
    Quaternion Inverted() const;

    /**
     * Invert this quaternion.
     */
    Quaternion& Invert();

    /**
     * Rotate a 3D vector with a quaternion.
     */
    Vector Transform(const Vector& v) const;

    /**
     * Extract rotation axis and angle from a quaternion.
     * @param   outAxis     Normalized rotation axis.
     * @param   outAngle    Rotation angle in radians.
     * @note    This is slow.
     */
    void ToAxis(Vector& outAxis, float& outAngle) const;

    /**
     * Create rotation matrix from quaternion.
     * @note Quaternion must be normalized.
     * @return Matrix representing the same rotation as input quaternion.
     */
    Matrix ToMatrix() const;

    /**
     * Convert quaternion to Euler angles.
     * @param   outPitch   Returned rotation in X axis (in radians).
     * @param   outYaw     Returned rotation in Y axis (in radians).
     * @param   outRoll    Returned rotation in Z axis (in radians).
     * @note    This is quite costly method.
     */
    void ToAngles(float& outPitch, float& outYaw, float& outRoll) const;

    /**
     * Spherical interpolation of two quaternions.
     * @param q0,q1 Quaternions to interpolate.
     * @param t     Interpolation factor.
     * @return Interpolated quaternion (equal to q0 when t=0.0f and equal to q1 when t=1.0f).
     */
    static Quaternion Interpolate(const Quaternion& q0, const Quaternion& q1, float t);

    /**
     * Check if two quaternions are almost equal.
     */
    static bool AlmostEqual(const Quaternion& a, const Quaternion& b, float epsilon = NFE_MATH_EPSILON);
};

} // namespace Math
} // namespace NFE


#include "QuaternionImpl.hpp"

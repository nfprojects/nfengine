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

NFE_ALIGN16
struct NFCOMMON_API Quaternion final
{
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

    NFE_INLINE Quaternion(float x, float y, float z, float w)
        : q(x, y, z, w)
    {}

    NFE_INLINE operator const Vector&() const { return q; }
    NFE_INLINE operator Vector&() { return q; }

    /**
     * Create null rotation quaternion.
     */
    NFE_INLINE static Quaternion Identity();

    /**
     * Create quaternion form axis and angle.
     */
    static Quaternion FromAxis(const Vector& axis, float angle);

    /**
     * Create quaternion representing rotation around X axis.
     */
    static Quaternion RotationX(float angle);

    /**
     * Create quaternion representing rotation around Y axis.
     */
    static Quaternion RotationY(float angle);

    /**
     * Create quaternion representing rotation around Z axis.
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
     * @param   yaw     Rotation in Y axis (in radians).
     * @param   pitch   Rotation in X axis (in radians).
     * @param   roll    Rotation in Z axis (in radians).
     */
    static Quaternion FromAngles(float yaw, float pitch, float roll);

    /**
     * Quaternion multiplication.
     */
    Quaternion operator * (const Quaternion& q2) const;
    NFE_INLINE Quaternion& operator *= (const Quaternion& q2);

    NFE_INLINE Quaternion& Normalize();
    NFE_INLINE Quaternion Normalized() const;

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
     * Extract rotation axis from a quaternion.
     * @note    This is slow.
     */
    Vector ToAxis() const;

    /**
     * Create rotation matrix from quaternion.
     * @note Quaternion must be normalized.
     * @return Matrix representing the same rotation as input quaternion.
     */
    Matrix ToMatrix() const;

    /**
     * Convert quaternion to Euler angles.
     * @param   outYaw     Returned rotation in Y axis (in radians).
     * @param   outPitch   Returned rotation in X axis (in radians).
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
};

} // namespace Math
} // namespace NFE

// TODO SSE version
#include "FPU/QuaternionFPU.hpp"

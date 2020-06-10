#pragma once

#include "Matrix4.hpp"

namespace NFE {
namespace Math {

/**
 * Class representing 4D quaternion. Useful for representing rotations.
 */
struct NFE_ALIGN(16) Quaternion final : public Common::Aligned<16>
{
    NFE_DECLARE_CLASS(Quaternion)

public:

    // XYZ - vector part, W - scalar part:
    // q = f[3] + i * q[0] + j * q[1] + k * q[2]
    Vec4f q;

    NFE_FORCE_INLINE Quaternion() : q(0.0f, 0.0f, 0.0f, 1.0f) { }
    NFE_FORCE_INLINE Quaternion(const Quaternion& rhs) : q(rhs.q) { }
    NFE_FORCE_INLINE explicit Quaternion(const Vec4f& v) : q(v) { }
    NFE_FORCE_INLINE explicit Quaternion(float i, float j, float k, float s) : q(i, j, k, s) { }

    NFE_FORCE_INLINE operator const Vec4f&() const { return q; }
    NFE_FORCE_INLINE operator Vec4f&() { return q; }

    NFE_FORCE_INLINE Quaternion& operator = (const Quaternion& rhs)
    {
        q = rhs.q;
        return *this;
    }

    // check if quaternion is normalized and its values are valid
    NFCOMMON_API bool IsValid() const;

    // Get transformed X, Y, Z axes.
    NFE_FORCE_INLINE const Vec4f GetAxisX() const;
    NFE_FORCE_INLINE const Vec4f GetAxisY() const;
    NFE_FORCE_INLINE const Vec4f GetAxisZ() const;

    // Create null rotation quaternion.
    NFE_FORCE_INLINE static const Quaternion Identity();

    // Create quaternion form axis and angle.
    // Note: Returned quaternion is normalized.
    NFCOMMON_API static const Quaternion FromAxisAndAngle(const Vec4f& axis, float angle);

    // Create quaternion representing rotation around X axis.
    // Note: Returned quaternion is normalized.
    NFCOMMON_API static const Quaternion RotationX(float angle);

    // Create quaternion representing rotation around Y axis.
    // Note: Returned quaternion is normalized.
    NFCOMMON_API static const Quaternion RotationY(float angle);

    // Create quaternion representing rotation around Z axis.
    // Note: Returned quaternion is normalized.
    NFCOMMON_API static const Quaternion RotationZ(float angle);

    // Quaternion multiplication
    NFCOMMON_API const Quaternion operator * (const Quaternion& q2) const;
    NFE_FORCE_INLINE Quaternion& operator *= (const Quaternion& q2);

    // Turn to unit quaternion (length = 1.0f).
    NFE_FORCE_INLINE Quaternion& Normalize();
    NFE_FORCE_INLINE const Quaternion Normalized() const;

    // Return conjugate of quaternion
    // Equals to Inverted() if quaternion is normalized.
    NFE_FORCE_INLINE const Quaternion Conjugate() const;

    // Calculate inverse of quaternion.
    NFCOMMON_API const Quaternion Inverted() const;

    // Invert this quaternion.
    NFCOMMON_API Quaternion& Invert();

    // Rotate a 3D vector with a quaternion.
    NFCOMMON_API const Vec4f TransformVector(const Vec4f& v) const;
    NFCOMMON_API const Vec3x8f TransformVector(const Vec3x8f& v) const;

    // Extract rotation axis and angle from a quaternion.
    // Note: This is slow.
    NFCOMMON_API void ToAxis(Vec4f& outAxis, float& outAngle) const;

    // Spherical interpolation of two quaternions.
    // Returns Interpolated quaternion (equal to q0 when t=0.0f and equal to q1 when t=1.0f).
    NFCOMMON_API static const Quaternion Interpolate(const Quaternion& q0, const Quaternion& q1, float t);

    // Check if two quaternions are almost equal.
    NFCOMMON_API static bool AlmostEqual(const Quaternion& a, const Quaternion& b, float epsilon = NFE_MATH_EPSILON);

    // Create quaternion from Euler angles.
    // pitch   - rotation in X axis (in radians)
    // yaw     - rotation in Y axis (in radians)
    // roll    - rotation in Z axis (in radians)
    NFCOMMON_API static const Quaternion FromEulerAngles(const Vec3f& angles);

    NFE_FORCE_INLINE static const Quaternion FromEulerAngles(float pitch, float yaw, float roll)
    {
        // TODO proper 'EulerAngles' class
        return FromEulerAngles(Vec3f(pitch, yaw, roll));
    }

    // Create quaternion from rotation matrix.
    NFCOMMON_API static const Quaternion FromMatrix(const Matrix4& matrix);

    // Convert quaternion to 4x4 matrix
    NFCOMMON_API const Matrix4 ToMatrix() const;

    // Convert quaternion to Euler angles (pitch, yaw, roll).
    // Note: This is quite costly method.
    NFCOMMON_API const Vec3f ToEulerAngles() const;
};


} // namespace Math
} // namespace NFE


#include "QuaternionImpl.hpp"

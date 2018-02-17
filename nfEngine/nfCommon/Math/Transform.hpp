/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Transform class declarations.
 */

#pragma once

#include "Math.hpp"
#include "Vector4.hpp"
#include "Quaternion.hpp"


namespace NFE {
namespace Math {

/**
 * Class representing 3D transformation (translation + rotation).
 */
class NFE_ALIGN(16) Transform final
{
public:
    Transform() = default;
    Transform(const Transform&) = default;
    Transform& operator = (const Transform&) = default;

    explicit Transform(const Vector4& translation, const Quaternion& rotation)
        : mTranslation(translation)
        , mRotation(rotation)
    { }

    const Vector4& GetTranslation() const { return mTranslation; }
    const Quaternion& GetRotation() const { return mRotation; }

    void SetTranslation(const Vector4& translation) { mTranslation = translation; }
    void SetRotation(const Quaternion& rotation) { mRotation = rotation; }

    /**
     * Transform composition.
     * The order is the same as for quaternions composition:
     * (a * b).Transform(x) == a.Transform(b.Transform(x))
     */
    NFCOMMON_API Transform operator * (const Transform& other) const;
    NFE_INLINE Transform& operator *= (const Transform& other);

    /**
     * Calculate inverse of quaternion.
     */
    NFCOMMON_API Transform Inverted() const;

    /**
     * Invert this quaternion.
     */
    NFCOMMON_API Transform& Invert();

    /**
     * Transform a 3D point.
     */
    NFCOMMON_API Vector4 TransformPoint(const Vector4& p) const;

    /**
     * Transform a 3D vector (direction).
     * @note Translation is ignored.
     */
    NFCOMMON_API Vector4 TransformVector(const Vector4& v) const;

    /**
     * Create matrix representing this transformation.
     */
    NFCOMMON_API Matrix4 ToMatrix() const;

    /**
     * Create transform from matrix.
     */
    NFCOMMON_API static Transform FromMatrix(const Matrix4& matrix);

    /**
     * Interpolate two transforms.
     * @note Translations are interpolated linearly, rotations - spherically.
     */
    NFCOMMON_API static Transform Interpolate(const Transform& t0, const Transform& t1, float t);

    /**
     * Check if two transforms are almost equal.
     */
    NFCOMMON_API static bool AlmostEqual(const Transform& a, const Transform& b, float epsilon = NFE_MATH_EPSILON);

private:
    Vector4 mTranslation;
    Quaternion mRotation;
};


Transform& Transform::operator *= (const Transform& other)
{
    *this = *this * other;
    return *this;
}


} // namespace Math
} // namespace NFE

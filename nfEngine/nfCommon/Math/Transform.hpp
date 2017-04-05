/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Transform class declarations.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"
#include "Quaternion.hpp"


namespace NFE {
namespace Math {

/**
 * Class representing 3D transformation (translation + rotation).
 */
class NFCOMMON_API NFE_ALIGN16 Transform final
{
public:
    Transform() = default;
    Transform(const Transform&) = default;
    Transform& operator = (const Transform&) = default;

    Transform(const Vector& translation, const Quaternion& rotation)
        : mTranslation(translation)
        , mRotation(rotation)
    { }

    NFE_INLINE const Vector& GetTranslation() const { return mTranslation; }
    NFE_INLINE const Quaternion& GetRotations() const { return mRotation; }

    NFE_INLINE void SetTranslation(const Vector& translation) { mTranslation = translation; }
    NFE_INLINE void SetRotation(const Quaternion& rotation) { mRotation = rotation; }

    /**
     * Transform composition.
     * The order is the same as for quaternions composition:
     * (a * b).Transform(x) == a.Transform(b.Transform(x))
     */
    Transform operator * (const Transform& other) const;
    NFE_INLINE Transform& operator *= (const Transform& other);

    /**
     * Calculate inverse of quaternion.
     */
    Transform Inverted() const;

    /**
     * Invert this quaternion.
     */
    Transform& Invert();

    /**
     * Transform a 3D point.
     */
    Vector TransformPoint(const Vector& p) const;

    /**
     * Transform a 3D vector (direction).
     * @note Translation is ignored.
     */
    Vector TransformVector(const Vector& v) const;

    /**
     * Create matrix representing this transformation.
     */
    Matrix ToMatrix() const;

    /**
     * Create transform from matrix.
     */
    static Transform FromMatrix(const Matrix& matrix);

    /**
     * Interpolate two transforms.
     * @note Translations are interpolated linearly, rotations - spherically.
     */
    static Transform Interpolate(const Transform& t0, const Transform& t1, float t);

    /**
     * Check if two transforms are almost equal.
     */
    static bool AlmostEqual(const Transform& a, const Transform& b, float epsilon = NFE_MATH_EPSILON);

private:
    Vector mTranslation;
    Quaternion mRotation;
};


Transform& Transform::operator *= (const Transform& other)
{
    *this = *this * other;
    return *this;
}


} // namespace Math
} // namespace NFE

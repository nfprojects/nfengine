#pragma once

#include "Quaternion.hpp"
#include "Box.hpp"
#include "Ray.hpp"

namespace NFE {
namespace Math {

// Class representing 3D transformation (translation + rotation).
class NFE_ALIGN(16) Transform final
{
    NFE_DECLARE_CLASS(Transform);

public:
    NFE_FORCE_INLINE Transform()
        : mTranslation(Vector4::Zero())
    { }

    Transform(const Transform&) = default;
    Transform& operator = (const Transform&) = default;

    NFE_FORCE_INLINE explicit Transform(const Vector4& translation, const Quaternion& rotation)
        : mTranslation(translation)
        , mRotation(rotation)
    { }

    NFE_FORCE_INLINE explicit Transform(const Vector4& translation)
        : mTranslation(translation)
    { }

    NFE_FORCE_INLINE explicit Transform(const Quaternion& rotation)
        : mRotation(rotation)
    { }

    NFE_FORCE_INLINE const Vector4& GetTranslation() const { return mTranslation; }
    NFE_FORCE_INLINE const Quaternion& GetRotation() const { return mRotation; }

    NFE_FORCE_INLINE void SetTranslation(const Vector4& translation) { mTranslation = translation; }
    NFE_FORCE_INLINE void SetRotation(const Quaternion& rotation) { mRotation = rotation; }

    NFE_FORCE_INLINE bool IsValid() const { return mTranslation.IsValid() && mRotation.IsValid(); }

    // Transform composition.
    // The order is the same as for quaternions composition:
    // (a * b).Transform(x) == a.Transform(b.Transform(x))
    NFCOMMON_API Transform operator * (const Transform& other) const;
    NFE_FORCE_INLINE Transform& operator *= (const Transform& other);

    // Calculate inverse of quaternion.
    NFCOMMON_API const Transform Inverted() const;

    // Invert this transform
    NFCOMMON_API Transform& Invert();

    // Transform a 3D point
    NFCOMMON_API const Vector4 TransformPoint(const Vector4& p) const;
    NFCOMMON_API const Vector3x8 TransformPoint(const Vector3x8& p) const;

    // Transform a 3D vector (direction).
    // Note: Translation is ignored.
    NFCOMMON_API const Vector4 TransformVector(const Vector4& v) const;
    NFCOMMON_API const Vector3x8 TransformVector(const Vector3x8& v) const;

    NFCOMMON_API const Box TransformBox(const Box& box) const;
    NFCOMMON_API const Ray TransformRay(const Ray& ray) const;

    // Convert transform to 4x4 matrix
    NFCOMMON_API const Matrix4 ToMatrix() const;

    // Create transform from 4x4 matrix.
    NFCOMMON_API static const Transform FromMatrix(const Matrix4& matrix);

    // Interpolate two transforms.
    // Note: Translations are interpolated linearly, rotations - spherically.
    NFCOMMON_API static const Transform Interpolate(const Transform& t0, const Transform& t1, float t);

    // Check if two transforms are almost equal.
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

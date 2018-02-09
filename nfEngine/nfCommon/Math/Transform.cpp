/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Transform function definitions.
 */

#include "PCH.hpp"
#include "Transform.hpp"


namespace NFE {
namespace Math {


Transform Transform::operator * (const Transform& other) const
{
    Transform result;
    result.mTranslation = mRotation.TransformVector(other.mTranslation) + mTranslation;
    result.mRotation = mRotation * other.mRotation;
    return result;
}

Transform Transform::Inverted() const
{
    Transform result;
    result.mRotation = mRotation.Inverted().Normalized();
    result.mTranslation = result.mRotation.TransformVector(-mTranslation);
    return result;
}

Transform& Transform::Invert()
{
    *this = Inverted();
    return *this;
}

Vector4 Transform::TransformPoint(const Vector4& p) const
{
    return mRotation.TransformVector(p) + mTranslation;
}

Vector4 Transform::TransformVector(const Vector4& v) const
{
    return mRotation.TransformVector(v);
}

Matrix Transform::ToMatrix() const
{
    Matrix result = mRotation.ToMatrix();
    result.r[3] = mTranslation;
    result.r[3][3] = 1.0f;
    return result;
}

Transform Transform::FromMatrix(const Matrix& matrix)
{
    return Transform(matrix.GetRow(3) & VECTOR_MASK_XYZ, Quaternion::FromMatrix(matrix));
}

Transform Transform::Interpolate(const Transform& t0, const Transform& t1, float t)
{
    return Transform(Vector4::Lerp(t0.mTranslation, t1.mTranslation, t), Quaternion::Interpolate(t0.mRotation, t1.mRotation, t));
}

bool Transform::AlmostEqual(const Transform& a, const Transform& b, float epsilon)
{
    if (!Vector4::AlmostEqual(a.mTranslation, b.mTranslation, epsilon))
        return false;

    return Quaternion::AlmostEqual(a.mRotation, b.mRotation, epsilon);
}

} // namespace Math
} // namespace NFE

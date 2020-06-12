#include "PCH.hpp"
#include "Transform.hpp"
#include "Vec3x8f.hpp"


namespace NFE {
namespace Math {


Transform Transform::operator * (const Transform& other) const
{
    Transform result;
    result.mTranslation = mRotation.TransformVector(other.mTranslation) + mTranslation;
    result.mRotation = mRotation * other.mRotation;
    return result;
}

const Transform Transform::Inverted() const
{
    Transform result;
    result.mRotation = mRotation.Conjugate();
    result.mTranslation = result.mRotation.TransformVector(-mTranslation);
    return result;
}

Transform& Transform::Invert()
{
    *this = Inverted();
    return *this;
}

const Vec4f Transform::TransformPoint(const Vec4f& p) const
{
    return mRotation.TransformVector(p) + mTranslation;
}

const Vec3x8f Transform::TransformPoint(const Vec3x8f& p) const
{
    return mRotation.TransformVector(p) + Vec3x8f(mTranslation);
}

const Vec4f Transform::TransformVector(const Vec4f& v) const
{
    return mRotation.TransformVector(v);
}

const Vec3x8f Transform::TransformVector(const Vec3x8f& v) const
{
    return mRotation.TransformVector(v);
}

const Box Transform::TransformBox(const Box& box) const
{
    // based on:
    // http://dev.theomader.com/transform-bounding-boxes/

    const Vec4f xa = mRotation.GetAxisX() * box.min.x;
    const Vec4f xb = mRotation.GetAxisX() * box.max.x;
    const Vec4f ya = mRotation.GetAxisY() * box.min.y;
    const Vec4f yb = mRotation.GetAxisY() * box.max.y;
    const Vec4f za = mRotation.GetAxisZ() * box.min.z;
    const Vec4f zb = mRotation.GetAxisZ() * box.max.z;

    return Box(
        Vec4f::Min(xa, xb) + Vec4f::Min(ya, yb) + Vec4f::Min(za, zb) + mTranslation,
        Vec4f::Max(xa, xb) + Vec4f::Max(ya, yb) + Vec4f::Max(za, zb) + mTranslation
    );
}

const Ray Transform::TransformRay(const Ray& ray) const
{
    const Vec4f origin = TransformPoint(ray.origin);
    const Vec4f dir = TransformVector(ray.dir);
    return Ray(origin, dir);
}

const Transform Transform::FromMatrix(const Matrix4& matrix)
{
    return Transform(matrix.GetRow(3) & Vec4f::MakeMask<1,1,1,0>(), Quaternion::FromMatrix(matrix));
}

const Transform Transform::Interpolate(const Transform& t0, const Transform& t1, float t)
{
    return Transform(Vec4f::Lerp(t0.mTranslation, t1.mTranslation, t), Quaternion::Interpolate(t0.mRotation, t1.mRotation, t));
}

bool Transform::AlmostEqual(const Transform& a, const Transform& b, float epsilon)
{
    if (!Vec4f::AlmostEqual(a.mTranslation, b.mTranslation, epsilon))
        return false;

    return Quaternion::AlmostEqual(a.mRotation, b.mRotation, epsilon);
}

const Matrix4 Transform::ToMatrix() const
{
    Matrix4 result = mRotation.ToMatrix();
    result.rows[3] = Vec4f(mTranslation.x, mTranslation.y, mTranslation.z, 1.0f);
    return result;
}

} // namespace Math
} // namespace NFE

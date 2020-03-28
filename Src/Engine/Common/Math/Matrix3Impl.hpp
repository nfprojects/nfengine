/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix3 inline functions definitions.
 */

#pragma once

#include "Matrix3.hpp"


namespace NFE {
namespace Math {


Vec3f& Matrix3::GetRow(int i)
{
    return reinterpret_cast<Vec3f&>(r[i]);
}

const Vec3f& Matrix3::GetRow(int i) const
{
    return reinterpret_cast<const Vec3f&>(r[i]);
}

Vec3f& Matrix3::operator[] (int i)
{
    return reinterpret_cast<Vec3f&>(r[i]);
}

const Vec3f& Matrix3::operator[] (int i) const
{
    return reinterpret_cast<const Vec3f&>(r[i]);
}

constexpr Matrix3::Matrix3()
    : r{ Vec3f(1.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f) }
{ }

constexpr Matrix3::Matrix3(const Vec3f& r0, const Vec3f& r1, const Vec3f& r2)
    : r{ r0, r1, r2 }
{ }

Matrix3::Matrix3(const std::initializer_list<float>& list)
{
    int i = 0;
    for (float x : list)
        f[i++] = x;
}

//////////////////////////////////////////////////////////////////////////

Matrix3 Matrix3::operator + (const Matrix3& b) const
{
    return Matrix3(*this) += b;
}

Matrix3 Matrix3::operator - (const Matrix3& b) const
{
    return Matrix3(*this) -= b;
}

Matrix3& Matrix3::operator += (const Matrix3& b)
{
    r[0] += b.r[0];
    r[1] += b.r[1];
    r[2] += b.r[2];
    return *this;
}

Matrix3& Matrix3::operator -= (const Matrix3& b)
{
    r[0] -= b.r[0];
    r[1] -= b.r[1];
    r[2] -= b.r[2];
    return *this;
}

Matrix3 Matrix3::operator * (float b) const
{
    return Matrix3(*this) *= b;
}

Matrix3 Matrix3::operator / (float b) const
{
    NFE_ASSERT(Math::Abs(b) > NFE_MATH_EPSILON, "Division by zero");

    return Matrix3(*this) /= b;
}

Matrix3& Matrix3::operator *= (float b)
{
    r[0] *= b;
    r[1] *= b;
    r[2] *= b;
    return *this;
}

Matrix3& Matrix3::operator /= (float b)
{
    NFE_ASSERT(Math::Abs(b) > NFE_MATH_EPSILON, "Division by zero");

    r[0] /= b;
    r[1] /= b;
    r[2] /= b;
    return *this;
}

constexpr bool Matrix3::operator == (const Matrix3& b) const
{
    return (r[0] == b.r[0]) && (r[1] == b.r[1]) && (r[2] == b.r[2]);
}

constexpr float Matrix3::Det() const
{
    return
        m[0][0] * m[1][1] * m[2][2]
        + m[0][1] * m[1][2] * m[2][0]
        + m[0][2] * m[1][0] * m[2][1]
        - m[0][2] * m[1][1] * m[2][0]
        - m[0][1] * m[1][0] * m[2][2]
        - m[0][0] * m[1][2] * m[2][1];
}

constexpr Matrix3 Matrix3::Transposed() const
{
    return Matrix3(
        Vec3f(m[0][0], m[1][0], m[2][0]),
        Vec3f(m[0][1], m[1][1], m[2][1]),
        Vec3f(m[0][2], m[1][2], m[2][2])
    );
}

Matrix3& Matrix3::Transpose()
{
    *this = Transposed();
    return *this;
}

Vec2f Matrix3::AffineTransform2(const Vec2f& a) const
{
    return a.x * r[0] + a.y * r[1] + r[2];
}

Vec3f Matrix3::LinearCombination(const Vec3f& a) const
{
    return a.x * r[0] + a.y * r[1] + a.z * r[2];
}

Matrix3 Matrix3::Abs(const Matrix3& m)
{
    return Matrix3(Vec3f::Abs(m[0]), Vec3f::Abs(m[1]), Vec3f::Abs(m[2]));
}

Vec3f operator* (const Vec3f& vector, const Matrix3& m)
{
    return m.LinearCombination(vector);
}


} // namespace Math
} // namespace NFE

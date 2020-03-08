/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix3 inline functions definitions.
 */

#pragma once

#include "Matrix3.hpp"


namespace NFE {
namespace Math {


Float3& Matrix3::GetRow(int i)
{
    return reinterpret_cast<Float3&>(r[i]);
}

const Float3& Matrix3::GetRow(int i) const
{
    return reinterpret_cast<const Float3&>(r[i]);
}

Float3& Matrix3::operator[] (int i)
{
    return reinterpret_cast<Float3&>(r[i]);
}

const Float3& Matrix3::operator[] (int i) const
{
    return reinterpret_cast<const Float3&>(r[i]);
}

constexpr Matrix3::Matrix3()
    : r{ Float3(1.0f, 0.0f, 0.0f), Float3(0.0f, 1.0f, 0.0f), Float3(0.0f, 0.0f, 1.0f) }
{ }

constexpr Matrix3::Matrix3(const Float3& r0, const Float3& r1, const Float3& r2)
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
        Float3(m[0][0], m[1][0], m[2][0]),
        Float3(m[0][1], m[1][1], m[2][1]),
        Float3(m[0][2], m[1][2], m[2][2])
    );
}

Matrix3& Matrix3::Transpose()
{
    *this = Transposed();
    return *this;
}

Float2 Matrix3::AffineTransform2(const Float2& a) const
{
    return a.x * r[0] + a.y * r[1] + r[2];
}

Float3 Matrix3::LinearCombination(const Float3& a) const
{
    return a.x * r[0] + a.y * r[1] + a.z * r[2];
}

Matrix3 Matrix3::Abs(const Matrix3& m)
{
    return Matrix3(Float3::Abs(m[0]), Float3::Abs(m[1]), Float3::Abs(m[2]));
}

Float3 operator* (const Float3& vector, const Matrix3& m)
{
    return m.LinearCombination(vector);
}


} // namespace Math
} // namespace NFE

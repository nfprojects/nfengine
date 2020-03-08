/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix2 inline functions definitions.
 */

#pragma once

#include "Matrix2.hpp"


namespace NFE {
namespace Math {


Float2& Matrix2::GetRow(int i)
{
    return reinterpret_cast<Float2&>(r[i]);
}

const Float2& Matrix2::GetRow(int i) const
{
    return reinterpret_cast<const Float2&>(r[i]);
}

Float2& Matrix2::operator[] (int i)
{
    return reinterpret_cast<Float2&>(r[i]);
}

const Float2& Matrix2::operator[] (int i) const
{
    return reinterpret_cast<const Float2&>(r[i]);
}

constexpr Matrix2::Matrix2()
    : r{ Float2(1.0f, 0.0f), Float2(0.0f, 1.0f) }
{ }

constexpr Matrix2::Matrix2(const Float2& r0, const Float2& r1)
    : r{ r0, r1 }
{ }

Matrix2::Matrix2(const std::initializer_list<float>& list)
{
    int i = 0;
    for (float x : list)
        f[i++] = x;
}

Matrix2 Matrix2::operator + (const Matrix2& b) const
{
    return Matrix2(*this) += b;
}

Matrix2 Matrix2::operator - (const Matrix2& b) const
{
    return Matrix2(*this) -= b;
}

Matrix2& Matrix2::operator += (const Matrix2& b)
{
    r[0] += b.r[0];
    r[1] += b.r[1];
    return *this;
}

Matrix2& Matrix2::operator -= (const Matrix2& b)
{
    r[0] -= b.r[0];
    r[1] -= b.r[1];
    return *this;
}

Matrix2 Matrix2::operator * (float b) const
{
    return Matrix2(*this) *= b;
}

Matrix2 Matrix2::operator / (float b) const
{
    NFE_ASSERT(Math::Abs(b) > NFE_MATH_EPSILON, "Division by zero");

    return Matrix2(*this) /= b;
}

Matrix2& Matrix2::operator *= (float b)
{
    r[0] *= b;
    r[1] *= b;
    return *this;
}

Matrix2& Matrix2::operator /= (float b)
{
    NFE_ASSERT(Math::Abs(b) > NFE_MATH_EPSILON, "Division by zero");

    r[0] /= b;
    r[1] /= b;
    return *this;
}

constexpr bool Matrix2::operator == (const Matrix2& b) const
{
    return (r[0] == b.r[0]) & (r[1] == b.r[1]);
}

constexpr float Matrix2::Det() const
{
    return m[0][0] * m[1][1] - m[0][1] * m[1][0];
}

Matrix2& Matrix2::operator *= (const Matrix2& b)
{
    *this = (*this) * b;
    return *this;
}

constexpr Matrix2 Matrix2::Transposed() const
{
    return Matrix2(Float2(m[0][0], m[1][0]), Float2(m[0][1], m[1][1]));
}

Matrix2& Matrix2::Transpose()
{
    *this = Transposed();
    return *this;
}

Float2 Matrix2::LinearCombination(const Float2& a) const
{
    return a.x * r[0] + a.y * r[1];
}

Matrix2 Matrix2::Abs(const Matrix2& m)
{
    return Matrix2(Float2::Abs(m[0]), Float2::Abs(m[1]));
}

Float2 operator* (const Float2& vector, const Matrix2& m)
{
    return m.LinearCombination(vector);
}


} // namespace Math
} // namespace NFE

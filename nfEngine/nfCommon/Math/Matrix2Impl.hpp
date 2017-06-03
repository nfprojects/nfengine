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
{
}

constexpr Matrix2::Matrix2(const Float2& r0, const Float2& r1)
    : r{ r0, r1 }
{
}

constexpr Matrix2::Matrix2(float r0c0, float r0c1, float r1c0, float r1c1)
    : r{ Float2(r0c0, r0c1), Float2(r1c0, r1c1) }
{
}

Matrix2::Matrix2(const std::initializer_list<float>& list)
{
    int i = 0;
    for (float x : list)
        f[i++] = x;
}

Matrix2 Matrix2::operator+ (const Matrix2& b) const
{
    return Matrix2(*this) += b;
}

Matrix2 Matrix2::operator- (const Matrix2& b) const
{
    return Matrix2(*this) -= b;
}

Matrix2& Matrix2::operator+= (const Matrix2& b)
{
    r[0] += b.r[0];
    r[1] += b.r[1];
    r[2] += b.r[2];
    r[3] += b.r[3];
    return *this;
}

Matrix2& Matrix2::operator-= (const Matrix2& b)
{
    r[0] -= b.r[0];
    r[1] -= b.r[1];
    r[2] -= b.r[2];
    r[3] -= b.r[3];
    return *this;
}

Matrix2 Matrix2::operator* (float b) const
{
    return Matrix2(*this) *= b;
}

Matrix2 Matrix2::operator/ (float b) const
{
    return Matrix2(*this) /= b;
}

Matrix2& Matrix2::operator*= (float b)
{
    r[0] *= b;
    r[1] *= b;
    r[2] *= b;
    r[3] *= b;
    return *this;
}

Matrix2& Matrix2::operator/= (float b)
{
    r[0] /= b;
    r[1] /= b;
    r[2] /= b;
    r[3] /= b;
    return *this;
}

bool Matrix2::operator== (const Matrix2& b) const
{
    int tmp0 = r[0] == b.r[0];
    int tmp1 = r[1] == b.r[1];
    int tmp2 = r[2] == b.r[2];
    int tmp3 = r[3] == b.r[3];
    return (tmp0 && tmp1) & (tmp2 && tmp3);
}

Matrix2& Matrix2::Invert()
{
    *this = Inverted();
    return *this;
}

Float2 Matrix2::LinearCombination2(const Float2& a) const
{
    return a.x * r[0] + a.y * r[1];
}

Matrix2 Matrix2::Abs(const Matrix2& m)
{
    return Matrix2(Float2::Abs(m[0]), Float2::Abs(m[1]));
}

bool Matrix2::Equal(const Matrix2& m1, const Matrix2& m2, float epsilon)
{
    Matrix2 diff = Abs(m1 - m2);
    Float2 epsilonV = Float2::Splat(epsilon);
    return ((diff[0] < epsilonV) && (diff[1] < epsilonV)) &&
        ((diff[2] < epsilonV) && (diff[3] < epsilonV));
}

Float2 operator* (const Float2& vector, const Matrix2& m)
{
    return m.LinearCombination(vector);
}


} // namespace Math
} // namespace NFE

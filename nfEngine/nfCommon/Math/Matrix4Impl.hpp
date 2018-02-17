/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix4 inline functions definitions.
 */

#pragma once

#include "Matrix4.hpp"


namespace NFE {
namespace Math {


Vector4& Matrix4::GetRow(int i)
{
    return reinterpret_cast<Vector4&>(r[i]);
}

const Vector4& Matrix4::GetRow(int i) const
{
    return reinterpret_cast<const Vector4&>(r[i]);
}

Vector4& Matrix4::operator[] (int i)
{
    return reinterpret_cast<Vector4&>(r[i]);
}

const Vector4& Matrix4::operator[] (int i) const
{
    return reinterpret_cast<const Vector4&>(r[i]);
}

Matrix4::Matrix4()
{
    r[0] = VECTOR_X;
    r[1] = VECTOR_Y;
    r[2] = VECTOR_Z;
    r[3] = VECTOR_W;
}

Matrix4::Matrix4(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3)
{
    r[0] = r0;
    r[1] = r1;
    r[2] = r2;
    r[3] = r3;
}

Matrix4::Matrix4(const std::initializer_list<float>& list)
{
    int i = 0;
    for (float x : list)
        f[i++] = x;
}

Matrix2 Matrix4::ToMatrix2() const
{
    Matrix2 ret;

    ret.m[0][0] = m[0][0];
    ret.m[0][1] = m[0][1];
    ret.m[1][0] = m[1][0];
    ret.m[1][1] = m[1][1];

    return ret;
}

Matrix3 Matrix4::ToMatrix3() const
{
    Matrix3 ret;

    ret.m[0][0] = m[0][0];
    ret.m[0][1] = m[0][1];
    ret.m[0][2] = m[0][2];
    ret.m[1][0] = m[1][0];
    ret.m[1][1] = m[1][1];
    ret.m[1][2] = m[1][2];
    ret.m[2][0] = m[2][0];
    ret.m[2][1] = m[2][1];
    ret.m[2][2] = m[2][2];

    return ret;
}

Matrix4 Matrix4::operator+ (const Matrix4& b) const
{
    return Matrix4(*this) += b;
}

Matrix4 Matrix4::operator- (const Matrix4& b) const
{
    return Matrix4(*this) -= b;
}

Matrix4& Matrix4::operator+= (const Matrix4& b)
{
    r[0] += b.r[0];
    r[1] += b.r[1];
    r[2] += b.r[2];
    r[3] += b.r[3];
    return *this;
}

Matrix4& Matrix4::operator-= (const Matrix4& b)
{
    r[0] -= b.r[0];
    r[1] -= b.r[1];
    r[2] -= b.r[2];
    r[3] -= b.r[3];
    return *this;
}

Matrix4 Matrix4::operator* (float b) const
{
    return Matrix4(*this) *= b;
}

Matrix4 Matrix4::operator/ (float b) const
{
    return Matrix4(*this) /= b;
}

Matrix4& Matrix4::operator*= (float b)
{
    r[0] *= b;
    r[1] *= b;
    r[2] *= b;
    r[3] *= b;
    return *this;
}

Matrix4& Matrix4::operator/= (float b)
{
    r[0] /= b;
    r[1] /= b;
    r[2] /= b;
    r[3] /= b;
    return *this;
}

bool Matrix4::operator== (const Matrix4& b) const
{
    int tmp0 = r[0] == b.r[0];
    int tmp1 = r[1] == b.r[1];
    int tmp2 = r[2] == b.r[2];
    int tmp3 = r[3] == b.r[3];
    return (tmp0 && tmp1) & (tmp2 && tmp3);
}

Matrix4& Matrix4::Invert()
{
    *this = Inverted();
    return *this;
}

Vector4 Matrix4::LinearCombination3(const Vector4& a) const
{
    const Vector4 tmp0 = Vector4::MulAndAdd(a.SplatX(), r[0], a.SplatY() * r[1]);
    const Vector4 tmp1 = Vector4::MulAndAdd(a.SplatZ(), r[2], r[3]);
    return tmp0 + tmp1;
}

Vector4 Matrix4::LinearCombination4(const Vector4& a) const
{
    const Vector4 tmp0 = Vector4::MulAndAdd(a.SplatX(), r[0], a.SplatY() * r[1]);
    const Vector4 tmp1 = Vector4::MulAndAdd(a.SplatZ(), r[2], a.SplatW() * r[3]);
    return tmp0 + tmp1;
}

Matrix4 Matrix4::Abs(const Matrix4& m)
{
    return Matrix4(Vector4::Abs(m[0]), Vector4::Abs(m[1]), Vector4::Abs(m[2]), Vector4::Abs(m[3]));
}

bool Matrix4::Equal(const Matrix4& m1, const Matrix4& m2, float epsilon)
{
    Matrix4 diff = Abs(m1 - m2);
    Vector4 epsilonV = Vector4::Splat(epsilon);
    return ((diff[0] < epsilonV) && (diff[1] < epsilonV)) &&
        ((diff[2] < epsilonV) && (diff[3] < epsilonV));
}

Matrix4 Matrix4::MakeTranslation3(const Vector4& pos)
{
    Matrix4 m;
    m.r[0] = VECTOR_X;
    m.r[1] = VECTOR_Y;
    m.r[2] = VECTOR_Z;
    m.r[3] = Vector4(pos.f[0], pos.f[1], pos.f[2], 1.0f);
    return m;
}

Vector4 operator* (const Vector4& vector, const Matrix4& m)
{
    return m.LinearCombination4(vector);
}


} // namespace Math
} // namespace NFE

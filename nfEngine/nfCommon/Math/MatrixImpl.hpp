/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix inline functions definitions.
 */

#pragma once

#include "Matrix.hpp"


namespace NFE {
namespace Math {


Vector& Matrix::GetRow(int i)
{
    return reinterpret_cast<Vector&>(r[i]);
}

const Vector& Matrix::GetRow(int i) const
{
    return reinterpret_cast<const Vector&>(r[i]);
}

Vector& Matrix::operator[] (int i)
{
    return reinterpret_cast<Vector&>(r[i]);
}

const Vector& Matrix::operator[] (int i) const
{
    return reinterpret_cast<const Vector&>(r[i]);
}

Matrix::Matrix()
{
    r[0] = VECTOR_X;
    r[1] = VECTOR_Y;
    r[2] = VECTOR_Z;
    r[3] = VECTOR_W;
}

Matrix::Matrix(const Vector& r0, const Vector& r1, const Vector& r2, const Vector& r3)
{
    r[0] = r0;
    r[1] = r1;
    r[2] = r2;
    r[3] = r3;
}

Matrix::Matrix(const std::initializer_list<float>& list)
{
    int i = 0;
    for (float x : list)
        f[i++] = x;
}

Matrix Matrix::operator+ (const Matrix& b) const
{
    return Matrix(*this) += b;
}

Matrix Matrix::operator- (const Matrix& b) const
{
    return Matrix(*this) -= b;
}

Matrix& Matrix::operator+= (const Matrix& b)
{
    r[0] += b.r[0];
    r[1] += b.r[1];
    r[2] += b.r[2];
    r[3] += b.r[3];
    return *this;
}

Matrix& Matrix::operator-= (const Matrix& b)
{
    r[0] -= b.r[0];
    r[1] -= b.r[1];
    r[2] -= b.r[2];
    r[3] -= b.r[3];
    return *this;
}

Matrix Matrix::operator* (float b) const
{
    return Matrix(*this) *= b;
}

Matrix Matrix::operator/ (float b) const
{
    return Matrix(*this) /= b;
}

Matrix& Matrix::operator*= (float b)
{
    r[0] *= b;
    r[1] *= b;
    r[2] *= b;
    r[3] *= b;
    return *this;
}

Matrix& Matrix::operator/= (float b)
{
    r[0] /= b;
    r[1] /= b;
    r[2] /= b;
    r[3] /= b;
    return *this;
}

bool Matrix::operator== (const Matrix& b) const
{
    int tmp0 = r[0] == b.r[0];
    int tmp1 = r[1] == b.r[1];
    int tmp2 = r[2] == b.r[2];
    int tmp3 = r[3] == b.r[3];
    return (tmp0 && tmp1) & (tmp2 && tmp3);
}

Matrix& Matrix::Invert()
{
    *this = Inverted();
    return *this;
}

Vector Matrix::LinearCombination3(const Vector& a) const
{
    const Vector tmp0 = Vector::MulAndAdd(a.SplatX(), r[0], a.SplatY() * r[1]);
    const Vector tmp1 = Vector::MulAndAdd(a.SplatZ(), r[2], r[3]);
    return tmp0 + tmp1;
}

Vector Matrix::LinearCombination4(const Vector& a) const
{
    const Vector tmp0 = Vector::MulAndAdd(a.SplatX(), r[0], a.SplatY() * r[1]);
    const Vector tmp1 = Vector::MulAndAdd(a.SplatZ(), r[2], a.SplatW() * r[3]);
    return tmp0 + tmp1;
}

Matrix Matrix::Abs(const Matrix& m)
{
    return Matrix(Vector::Abs(m[0]), Vector::Abs(m[1]), Vector::Abs(m[2]), Vector::Abs(m[3]));
}

bool Matrix::Equal(const Matrix& m1, const Matrix& m2, float epsilon)
{
    Matrix diff = Abs(m1 - m2);
    Vector epsilonV = Vector::Splat(epsilon);
    return ((diff[0] < epsilonV) && (diff[1] < epsilonV)) &&
        ((diff[2] < epsilonV) && (diff[3] < epsilonV));
}

Matrix Matrix::MakeTranslation3(const Vector& pos)
{
    Matrix m;
    m.r[0] = VECTOR_X;
    m.r[1] = VECTOR_Y;
    m.r[2] = VECTOR_Z;
    m.r[3] = Vector(pos.f[0], pos.f[1], pos.f[2], 1.0f);
    return m;
}

Vector operator* (const Vector& vector, const Matrix& m)
{
    return m.LinearCombination4(vector);
}


} // namespace Math
} // namespace NFE

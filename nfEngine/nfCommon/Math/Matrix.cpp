/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix function definitions.
 */

#include "PCH.hpp"
#include "Matrix.hpp"


namespace NFE {
namespace Math {

Matrix Matrix::MakeLookTo(const Vector& EyePosition, const Vector& EyeDirection,
                    const Vector& UpDirection)
{
    Vector zaxis = EyeDirection.Normalized3();
    Vector xaxis = Vector::Cross3(UpDirection, zaxis).Normalized3();
    Vector yaxis = Vector::Cross3(zaxis, xaxis);

    return Matrix(Vector(xaxis.f[0], yaxis.f[0], zaxis.f[0], 0.0f),
                  Vector(xaxis.f[1], yaxis.f[1], zaxis.f[1], 0.0f),
                  Vector(xaxis.f[2], yaxis.f[2], zaxis.f[2], 0.0f),
                  Vector(-Vector::Dot3(xaxis, EyePosition),
                         -Vector::Dot3(yaxis, EyePosition),
                         -Vector::Dot3(zaxis, EyePosition),
                         1.0f));
}

Matrix Matrix::MakePerspective(float aspect, float fovY, float farZ, float nearZ)
{
    float yScale = 1.0f / tanf(fovY * 0.5f);
    float xScale = yScale / aspect;

    return Matrix(Vector(xScale, 0.0f,   0.0f,                           0.0f),
                  Vector(0.0f,   yScale, 0.0f,                           0.0f),
                  Vector(0.0f,   0.0f,   farZ / (farZ - nearZ),          1.0f),
                  Vector(0.0f,   0.0f,   -nearZ * farZ / (farZ - nearZ), 0.0f));
}

Matrix Matrix::MakeOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    return Matrix(
               Vector(2.0f / (right - left), 0.0f,                  0.0f,                  0.0f),
               Vector(0.0f,                  2.0f / (top - bottom), 0.0f,                  0.0f),
               Vector(0.0f,                  0.0f,                  1.0f / (zFar - zNear), 0.0f),
               Vector((left + right) / (left - right),
                      (top + bottom) / (bottom - top),
                      zNear / (zNear - zFar),
                      1.0f));
}

Matrix Matrix::MakeScaling(const Vector& scale)
{
    return Matrix(Vector(scale.f[0], 0.0f, 0.0f, 0.0f),
                  Vector(0.0f, scale.f[1], 0.0f, 0.0f),
                  Vector(0.0f, 0.0f, scale.f[2], 0.0f),
                  Vector(0.0f, 0.0f, 0.0f, 1.0f));
}

Matrix Matrix::operator* (const Matrix& b) const
{
    return Matrix(b.LinearCombination4(r[0]),
                  b.LinearCombination4(r[1]),
                  b.LinearCombination4(r[2]),
                  b.LinearCombination4(r[3]));
}

Matrix& Matrix::operator*= (const Matrix& b)
{
    r[0] = b.LinearCombination4(r[0]);
    r[1] = b.LinearCombination4(r[1]);
    r[2] = b.LinearCombination4(r[2]);
    r[3] = b.LinearCombination4(r[3]);
    return *this;
}

} // namespace Math
} // namespace NFE

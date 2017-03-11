/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix function definitions.
 */

#include "PCH.hpp"
#include "Matrix.hpp"

namespace NFE {
namespace Math {

Matrix MatrixLookTo(const Vector& EyePosition, const Vector& EyeDirection,
                    const Vector& UpDirection)
{
    Vector zaxis = VectorNormalize3(EyeDirection);
    Vector xaxis = VectorNormalize3(VectorCross3(UpDirection, zaxis));
    Vector yaxis = VectorCross3(zaxis, xaxis);

    return Matrix(Vector(xaxis.f[0], yaxis.f[0], zaxis.f[0], 0.0f),
                  Vector(xaxis.f[1], yaxis.f[1], zaxis.f[1], 0.0f),
                  Vector(xaxis.f[2], yaxis.f[2], zaxis.f[2], 0.0f),
                  Vector(-VectorDot3(xaxis, EyePosition)[0],
                         -VectorDot3(yaxis, EyePosition)[0],
                         -VectorDot3(zaxis, EyePosition)[0],
                         1.0f));
}

Matrix MatrixPerspective(float aspect, float fovY, float farZ, float nearZ)
{
    float yScale = 1.0f / tanf(fovY * 0.5f);
    float xScale = yScale / aspect;

    return Matrix(Vector(xScale, 0.0f,   0.0f,                           0.0f),
                  Vector(0.0f,   yScale, 0.0f,                           0.0f),
                  Vector(0.0f,   0.0f,   farZ / (farZ - nearZ),          1.0f),
                  Vector(0.0f,   0.0f,   -nearZ * farZ / (farZ - nearZ), 0.0f));
}

Matrix MatrixOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
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

Matrix MatrixScaling(const Vector& scale)
{
    return Matrix(Vector(scale.f[0], 0.0f, 0.0f, 0.0f),
                  Vector(0.0f, scale.f[1], 0.0f, 0.0f),
                  Vector(0.0f, 0.0f, scale.f[2], 0.0f),
                  Vector(0.0f, 0.0f, 0.0f, 1.0f));
}

Matrix Matrix::operator* (const Matrix& b) const
{
    return Matrix(LinearCombination(r[0], b),
                  LinearCombination(r[1], b),
                  LinearCombination(r[2], b),
                  LinearCombination(r[3], b));
}

Matrix& Matrix::operator*= (const Matrix& b)
{
    r[0] = LinearCombination(r[0], b);
    r[1] = LinearCombination(r[1], b);
    r[2] = LinearCombination(r[2], b);
    r[3] = LinearCombination(r[3], b);
    return *this;
}

} // namespace Math
} // namespace NFE

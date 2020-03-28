/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix3 definitions.
 */

#include "PCH.hpp"
#include "Matrix3.hpp"


namespace NFE {
namespace Math {


bool Matrix3::Equal(const Matrix3& m1, const Matrix3& m2, float epsilon)
{
    const Matrix3 diff = Abs(m1 - m2);
    const Vec3f epsilonV = Vec3f::Splat(epsilon);
    return (diff[0] < epsilonV) && (diff[1] < epsilonV) && (diff[2] < epsilonV);
}

Matrix3 Matrix3::operator * (const Matrix3& b) const
{
    return Matrix3(b.LinearCombination(r[0]), b.LinearCombination(r[1]), b.LinearCombination(r[2]));
}

bool Matrix3::Invert(Matrix3& outInverted) const
{
    const float det = Det();
    if (Math::Abs(det) < NFE_MATH_EPSILON)
    {
        return false;
    }

    Matrix3 temp;
    temp.m[0][0] = m[1][1] * m[2][2] - m[2][1] * m[1][2];
    temp.m[0][1] = m[0][2] * m[2][1] - m[0][1] * m[2][2];
    temp.m[0][2] = m[0][1] * m[1][2] - m[0][2] * m[1][1];
    temp.m[1][0] = m[1][2] * m[2][0] - m[1][0] * m[2][2];
    temp.m[1][1] = m[0][0] * m[2][2] - m[0][2] * m[2][0];
    temp.m[1][2] = m[1][0] * m[0][2] - m[0][0] * m[1][2];
    temp.m[2][0] = m[1][0] * m[2][1] - m[2][0] * m[1][1];
    temp.m[2][1] = m[2][0] * m[0][1] - m[0][0] * m[2][1];
    temp.m[2][2] = m[0][0] * m[1][1] - m[1][0] * m[0][1];

    outInverted = temp / det;
    return true;
}


} // namespace Math
} // namespace NFE

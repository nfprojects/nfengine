/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix2 definitions.
 */

#include "PCH.hpp"
#include "Matrix2.hpp"
#include "Transcendental.hpp"


namespace NFE {
namespace Math {


bool Matrix2::Equal(const Matrix2& m1, const Matrix2& m2, float epsilon)
{
    const Matrix2 diff = Abs(m1 - m2);
    const Vec2f epsilonV = Vec2f::Splat(epsilon);
    return (diff[0] < epsilonV) && (diff[1] < epsilonV);
}

Matrix2 Matrix2::operator * (const Matrix2& b) const
{
    return Matrix2(b.LinearCombination(r[0]), b.LinearCombination(r[1]));
}

bool Matrix2::Invert(Matrix2& outInverted) const
{
    const float det = Det();
    if (Math::Abs(det) < NFE_MATH_EPSILON)
    {
        return false;
    }

    const Matrix2 temp(Vec2f(m[1][1], -m[0][1]), Vec2f(-m[1][0], m[0][0]));
    outInverted = temp / det;
    return true;
}

Matrix2 Matrix2::MakeRotation(float angle)
{
    const float c = Cos(angle);
    const float s = Sin(angle);
    return Matrix2(Vec2f(c, -s), Vec2f(s, c));
}

} // namespace Math
} // namespace NFE

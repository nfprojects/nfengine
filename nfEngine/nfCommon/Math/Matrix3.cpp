/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Matrix3 definitions.
 */

#pragma once

#include "PCH.hpp"
#include "Matrix3.hpp"


namespace NFE {
namespace Math {


bool Matrix3::Equal(const Matrix3& m1, const Matrix3& m2, float epsilon)
{
    const Matrix3 diff = Abs(m1 - m2);
    const Float3 epsilonV = Float3::Splat(epsilon);
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

    const Matrix3 temp;
    // TODO

    outInverted = temp / det;
    return true;
}


} // namespace Math
} // namespace NFE

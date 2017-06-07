/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Triangle class definition.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"

namespace NFE {
namespace Math {

class NFE_ALIGN(16) Triangle
{
public:
    Vector v0, v1, v2; //< Triangle vertices

    NFE_INLINE Triangle() : v0(), v1(), v2() {}
    NFE_INLINE Triangle(const Vector& v0_, const Vector& v1_, const Vector& v2_)
        : v0(v0_), v1(v1_), v2(v2_) {}
};

} // namespace Math
} // namespace NFE

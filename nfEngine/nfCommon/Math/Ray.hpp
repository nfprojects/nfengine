/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Ray class definition.
 */

#pragma once

#include "Math.hpp"
#include "Vector.hpp"

namespace NFE {
namespace Math {

class NFE_ALIGN(16) Ray
{
public:
    Vector dir;
    Vector invDir;
    Vector origin;

    Ray() {}
    Ray(const Vector& direction, const Vector& origin)
        : origin(origin)
    {
        dir = direction.Normalized3();
        invDir = Vector::Reciprocal(dir);
    }
};

class NFE_ALIGN(16) RayBoxSegment
{
public:
    Vector nearDist;
    Vector farDist;
};

} // namespace Math
} // namespace NFE

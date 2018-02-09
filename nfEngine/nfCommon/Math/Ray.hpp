/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Ray class definition.
 */

#pragma once

#include "Math.hpp"
#include "Vector4.hpp"

namespace NFE {
namespace Math {

class NFE_ALIGN(16) Ray
{
public:
    Vector4 dir;
    Vector4 invDir;
    Vector4 origin;

    Ray() {}
    explicit Ray(const Vector4& direction, const Vector4& origin)
        : origin(origin)
    {
        dir = direction.Normalized3();
        invDir = Vector4::Reciprocal(dir);
    }
};

class NFE_ALIGN(16) RayBoxSegment
{
public:
    Vector4 nearDist;
    Vector4 farDist;
};

} // namespace Math
} // namespace NFE

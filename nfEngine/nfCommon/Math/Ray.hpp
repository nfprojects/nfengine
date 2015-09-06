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

class NFE_ALIGN16 Ray
{
public:
    Vector dir;
    Vector invDir;
    Vector origin;
};

class NFE_ALIGN16 RayBoxSegment
{
public:
    Vector nearDist;
    Vector farDist;
};

} // namespace Math
} // namespace NFE

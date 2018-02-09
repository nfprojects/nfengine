/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  FPU-specific geometry function definitions.
 */

#include "PCH.hpp"
#include "../Geometry.hpp"
#include "../Box.hpp"

#ifndef NFE_USE_SSE

namespace NFE {
namespace Math {

// Box-box intersection test
template<> NFCOMMON_API
bool Intersect(const Box& box1, const Box& box2)
{
    return Vector4::Less3(box1.min, box2.max) && Vector4::Less3(box2.min, box1.max);
}

// Box-point intersection test
template<> NFCOMMON_API
bool Intersect(const Box& box, const Vector4& point)
{
    return Vector4::GreaterEq3(box.max, point) && Vector4::GreaterEq3(point, box.min);
}

template<> NFCOMMON_API
IntersectionResult IntersectEx(const Box& box1, const Box& box2)
{
    if (Vector4::GreaterMask(box1.min, box2.max) & 0x7)
        return IntersectionResult::Outside;
    if (Vector4::GreaterMask(box2.min, box1.max) & 0x7)
        return IntersectionResult::Outside;

    if (Vector4::Greater3(box1.min, box2.min) && Vector4::Greater3(box2.max, box1.max))
        return IntersectionResult::Inside;

    return IntersectionResult::Intersect;
}

} // namespace Math
} // namespace NFE

#endif // NFE_USE_SSE
/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  FPU-specific geometry function definitions.
 */

#include "../../PCH.hpp"
#include "../Geometry.hpp"
#include "../Box.hpp"

#ifndef NFE_USE_SSE

namespace NFE {
namespace Math {

// Box-box intersection test
template<> NFCOMMON_API
bool Intersect(const Box& box1, const Box& box2)
{
    return VectorLess3(box1.min, box2.max) && VectorLess3(box2.min, box1.max);
}

// Box-point intersection test
template<> NFCOMMON_API
bool Intersect(const Box& box, const Vector& point)
{
    return VectorGreaterEq3(box.max, point) && VectorGreaterEq3(point, box.min);
}

} // namespace Math
} // namespace NFE

#endif // NFE_USE_SSE
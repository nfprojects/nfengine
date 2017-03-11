/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  SSE-specific geometry function definitions.
 */

#include "PCH.hpp"
#include "../Geometry.hpp"
#include "../Box.hpp"


#ifdef NFE_USE_SSE

namespace NFE {
namespace Math {

// Box-box intersection test
template<> NFCOMMON_API
bool Intersect(const Box& box1, const Box& box2)
{
    Vector temp0 = _mm_cmpge_ps(box1.min, box2.max);
    Vector temp1 = _mm_cmpge_ps(box2.min, box1.max);
    return (_mm_movemask_ps(_mm_or_ps(temp0, temp1)) & 7) == 0;
}

// Box-point intersection test
template<> NFCOMMON_API
bool Intersect(const Box& box, const Vector& point)
{
    Vector cmpMax = _mm_cmpge_ps(box.max, point);
    Vector cmpMin = _mm_cmpge_ps(point, box.min);
    return (_mm_movemask_ps(_mm_and_ps(cmpMax, cmpMin)) & 7) == 7;
}

template<> NFCOMMON_API
IntersectionResult IntersectEx(const Box& box1, const Box& box2)
{
    Vector temp0 = _mm_cmpge_ps(box1.min, box2.max);
    Vector temp1 = _mm_cmpge_ps(box2.min, box1.max);
    if (_mm_movemask_ps(_mm_or_ps(temp0, temp1)) & 7)
        return IntersectionResult::Outside;

    temp0 = _mm_cmpge_ps(box1.min, box2.min);
    temp1 = _mm_cmpge_ps(box2.max, box1.max);
    if ((_mm_movemask_ps(_mm_and_ps(temp0, temp1)) & 7) == 7)
        return IntersectionResult::Inside;

    return IntersectionResult::Intersect;
}

} // namespace Math
} // namespace NFE

#endif // NFE_USE_SSE
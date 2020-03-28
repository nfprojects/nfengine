#pragma once

#include "Vec3x8f.hpp"
#include "Triangle.hpp"


namespace NFE {
namespace Math {

/**
 * 8 triangles (SIMD version).
 */
class NFE_ALIGN(32) Triangle_Simd8 : public Common::Aligned<32>
{
public:
    Vec3x8f v0;
    Vec3x8f edge1;
    Vec3x8f edge2;

    Triangle_Simd8() = default;
    Triangle_Simd8(const Triangle_Simd8&) = default;
    Triangle_Simd8& operator = (const Triangle_Simd8&) = default;

    // splat single triangle
    NFE_FORCE_INLINE explicit Triangle_Simd8(const Triangle& tri)
        : v0(tri.v0)
        , edge1(tri.v1 - tri.v0)
        , edge2(tri.v2 - tri.v0)
    { }
};


} // namespace Math
} // namespace NFE

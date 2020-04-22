#pragma once

#include "Triangle.hpp"


namespace NFE {
namespace Math {


template<typename VecType>
class NFE_ALIGN(alignof(VecType)) SimdTriangle : public Common::Aligned<alignof(VecType)>
{
public:

    VecType v0;
    VecType edge1;
    VecType edge2;

    SimdTriangle() = default;
    SimdTriangle(const SimdTriangle&) = default;
    SimdTriangle& operator = (const SimdTriangle&) = default;

    // splat single triangle
    NFE_FORCE_INLINE explicit SimdTriangle(const Triangle& tri)
        : v0(tri.v0)
        , edge1(tri.v1 - tri.v0)
        , edge2(tri.v2 - tri.v0)
    { }
};


} // namespace Math
} // namespace NFE

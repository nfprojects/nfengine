#pragma once

#include "Math.hpp"
#include "Vec4f.hpp"
#include "Vec3f.hpp"


namespace NFE {
namespace Math {


class NFE_ALIGN(16) Triangle
{
public:
    Vec4f v0, v1, v2;

    NFE_FORCE_INLINE Triangle() = default;
    NFE_FORCE_INLINE Triangle(const Triangle&) = default;

    NFE_FORCE_INLINE Triangle(const Vec4f& v0, const Vec4f& v1, const Vec4f& v2)
        : v0(v0), v1(v1), v2(v2)
    {}
};

class ProcessedTriangle
{
public:
    Vec3f v0;
    Vec3f edge1;
    Vec3f edge2;

    NFE_FORCE_INLINE ProcessedTriangle() = default;
    NFE_FORCE_INLINE ProcessedTriangle(const ProcessedTriangle&) = default;

    NFE_FORCE_INLINE ProcessedTriangle(const Vec4f& v0, const Vec4f& v1, const Vec4f& v2)
    {
        this->v0 = v0.ToVec3f();
        edge1 = (v1 - v0).ToVec3f();
        edge2 = (v2 - v0).ToVec3f();
    }
};


} // namespace Math
} // namespace NFE

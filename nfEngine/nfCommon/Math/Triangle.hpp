#pragma once

#include "Math.hpp"
#include "Vector4.hpp"
#include "Float3.hpp"


namespace NFE {
namespace Math {


class NFE_ALIGN(16) Triangle
{
public:
    Vector4 v0, v1, v2;

    NFE_FORCE_INLINE Triangle() = default;
    NFE_FORCE_INLINE Triangle(const Triangle&) = default;

    NFE_FORCE_INLINE Triangle(const Vector4& v0, const Vector4& v1, const Vector4& v2)
        : v0(v0), v1(v1), v2(v2)
    {}
};

class ProcessedTriangle
{
public:
    Float3 v0;
    Float3 edge1;
    Float3 edge2;

    NFE_FORCE_INLINE ProcessedTriangle() = default;
    NFE_FORCE_INLINE ProcessedTriangle(const ProcessedTriangle&) = default;

    NFE_FORCE_INLINE ProcessedTriangle(const Vector4& v0, const Vector4& v1, const Vector4& v2)
    {
        this->v0 = v0.ToFloat3();
        edge1 = (v1 - v0).ToFloat3();
        edge2 = (v2 - v0).ToFloat3();
    }
};


} // namespace Math
} // namespace NFE

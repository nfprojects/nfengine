#pragma once

#include "Vector4.hpp"
#include "../Reflection/ReflectionClassMacros.hpp"

namespace NFE {
namespace Math {

class HdrColorRGB 
{
    NFE_DECLARE_CLASS(HdrColorRGB);

public:
    float r;
    float g;
    float b;

    NFE_FORCE_INLINE constexpr HdrColorRGB() : r(0.0f), g(0.0f), b(0.0f) { }
    NFE_FORCE_INLINE constexpr HdrColorRGB(float r, float g, float b) : r(r), g(g), b(b) { }

    NFE_FORCE_INLINE const Vector4 ToVector4() const
    {
        return Vector4(r, g, b, 1.0f);
    }
};

} // namespace Math
} // namespace NFE

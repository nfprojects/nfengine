#pragma once

#include "ColorHelpers.hpp"
#include "../Reflection/ReflectionClassDeclare.hpp"

namespace NFE {
namespace Math {

class LdrColorRGB;

// High dynamic range color
// sRGB (Rec. 709) color space, linear
class HdrColorRGB 
{
    NFE_DECLARE_CLASS(HdrColorRGB);

public:
    float r;
    float g;
    float b;

    NFE_FORCE_INLINE constexpr HdrColorRGB() : r(0.0f), g(0.0f), b(0.0f) { }
    NFE_FORCE_INLINE constexpr HdrColorRGB(float r, float g, float b) : r(r), g(g), b(b) { }
    NFE_FORCE_INLINE constexpr HdrColorRGB(const Vector4& color) : r(color.x), g(color.y), b(color.z) { }
    NFE_FORCE_INLINE constexpr HdrColorRGB(float intensity) : r(intensity), g(intensity), b(intensity) { }
    NFCOMMON_API HdrColorRGB(const LdrColorRGB& color);

    NFE_FORCE_INLINE const Vector4 ToVector4() const
    {
        return Vector4(r, g, b, 1.0f);
    }

    NFE_FORCE_INLINE bool IsBlack() const
    {
        return r <= 0.0f && g <= 0.0f && b <= 0.0f;
    }

    NFE_FORCE_INLINE float Luminance() const
    {
        return Math::Vector4::Dot3(ToVector4(), Math::c_rgbIntensityWeights);
    }

    NFE_FORCE_INLINE bool IsValid() const
    {
        return r >= 0.0f && g >= 0.0f && b >= 0.0f &&
            Math::IsValid(r) && Math::IsValid(g) && Math::IsValid(b);
    }
};

} // namespace Math
} // namespace NFE

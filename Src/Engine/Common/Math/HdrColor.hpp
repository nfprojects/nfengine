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
    NFE_DECLARE_CLASS(HdrColorRGB)

public:
    float r;
    float g;
    float b;

    NFE_FORCE_INLINE constexpr HdrColorRGB() : r(0.0f), g(0.0f), b(0.0f) { }
    NFE_FORCE_INLINE constexpr HdrColorRGB(float r, float g, float b) : r(r), g(g), b(b) { }
    NFE_FORCE_INLINE constexpr HdrColorRGB(const Vec4f& color) : r(color.x), g(color.y), b(color.z) { }
    NFE_FORCE_INLINE constexpr HdrColorRGB(float intensity) : r(intensity), g(intensity), b(intensity) { }
    NFCOMMON_API HdrColorRGB(const LdrColorRGB& color);

    NFE_FORCE_INLINE const Vec4f ToVec4f() const
    {
        return Vec4f(r, g, b, 1.0f);
    }

    NFE_FORCE_INLINE bool IsBlack() const
    {
        return r <= 0.0f && g <= 0.0f && b <= 0.0f;
    }

    NFE_FORCE_INLINE float Luminance() const
    {
        return Math::Vec4f::Dot3(ToVec4f(), Math::c_rgbIntensityWeights);
    }

    NFE_FORCE_INLINE bool IsValid() const
    {
        return r >= 0.0f && g >= 0.0f && b >= 0.0f &&
            Math::IsValid(r) && Math::IsValid(g) && Math::IsValid(b);
    }
};

// High dynamic range color
// sRGB (Rec. 709) color space, linear
class HdrColorRGBA : public HdrColorRGB
{
    NFE_DECLARE_CLASS(HdrColorRGBA)

public:
    float a;

    NFE_FORCE_INLINE constexpr HdrColorRGBA() : HdrColorRGB(), a(1.0f) { }
    NFE_FORCE_INLINE constexpr HdrColorRGBA(float r, float g, float b, float a = 1.0f) : HdrColorRGB(r, g, b), a(a) { }
    NFE_FORCE_INLINE constexpr HdrColorRGBA(const Vec4f& color) : HdrColorRGB(color), a(color.w) { }
    NFE_FORCE_INLINE constexpr HdrColorRGBA(float intensity) : HdrColorRGB(intensity), a(1.0f) { }

    NFE_FORCE_INLINE const Vec4f ToVec4f() const
    {
        return Vec4f(r, g, b, a);
    }

    NFE_FORCE_INLINE bool IsBlack() const
    {
        return r <= 0.0f && g <= 0.0f && b <= 0.0f;
    }

    NFE_FORCE_INLINE float Luminance() const
    {
        return Math::Vec4f::Dot3(ToVec4f(), Math::c_rgbIntensityWeights);
    }

    NFE_FORCE_INLINE bool IsValid() const
    {
        return r >= 0.0f && g >= 0.0f && b >= 0.0f && a >= 0.0f && a <= 1.0f &&
            Math::IsValid(r) && Math::IsValid(g) && Math::IsValid(b) && Math::IsValid(a);
    }
};

} // namespace Math
} // namespace NFE

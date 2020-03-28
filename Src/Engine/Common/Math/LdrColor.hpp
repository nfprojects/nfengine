#pragma once

#include "Vec4f.hpp"
#include "../Reflection/ReflectionClassDeclare.hpp"

namespace NFE {
namespace Math {

// Low dynamic range color
// sRGB (Rec. 709) color space and gamma
class LdrColorRGB
{
    NFE_DECLARE_CLASS(LdrColorRGB)

public:
    uint8 r;
    uint8 g;
    uint8 b;

    NFE_FORCE_INLINE constexpr LdrColorRGB(uint8 r, uint8 g, uint8 b) : r(r), g(g), b(b) { }

    NFE_FORCE_INLINE const Vec4f AsVec4f() const
    {
        // TODO can optimize
        return Vec4f(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    }

    NFE_FORCE_INLINE bool IsBlack() const
    {
        return r == 0 && g == 0 && b == 0;
    }

    static constexpr const LdrColorRGB White()      { return { 255, 255, 255 }; }
    static constexpr const LdrColorRGB Silver()     { return { 0xC0, 0xC0, 0xC0 }; }
    static constexpr const LdrColorRGB Gray()       { return { 0x80, 0x80, 0x80 }; }
    static constexpr const LdrColorRGB Black()      { return { 0, 0, 0 }; }
    static constexpr const LdrColorRGB Red()        { return { 255, 0, 0 }; }
    static constexpr const LdrColorRGB Maroon()     { return { 0x80, 0, 0 }; }
    static constexpr const LdrColorRGB Lime()       { return { 0, 255, 0 }; }
    static constexpr const LdrColorRGB Green()      { return { 0, 255, 0 }; }
    static constexpr const LdrColorRGB Blue()       { return { 0, 0, 255 }; }
    static constexpr const LdrColorRGB Navy()       { return { 0, 0, 0x80 }; }
    static constexpr const LdrColorRGB Yellow()     { return { 255, 255, 0 }; }
    static constexpr const LdrColorRGB Olive()      { return { 0x80, 0x80, 0 }; }
    static constexpr const LdrColorRGB Cyan()       { return { 0, 255, 255 }; }
    static constexpr const LdrColorRGB Teal()       { return { 0, 0x80, 0x80 }; }
    static constexpr const LdrColorRGB Magenta()    { return { 255, 0, 255 }; }
    static constexpr const LdrColorRGB Purple()     { return { 0x80, 0, 0x80 }; }
};

// Low dynamic range color
// RGB channels: sRGB (Rec. 709) color space and gamma
// Alpha channel: linear
class LdrColorRGBA : public LdrColorRGB
{
    NFE_DECLARE_CLASS(LdrColorRGBA)

public:

    uint8 a;

    NFE_FORCE_INLINE constexpr LdrColorRGBA(const LdrColorRGB rgb, uint8 a = 255u)
        : LdrColorRGB(rgb)
        , a(a)
    { }

    NFE_FORCE_INLINE constexpr LdrColorRGBA(uint8 r, uint8 g, uint8 b, uint8 a = 255u)
        : LdrColorRGB(r, g, b)
        , a(a)
    { }

    static constexpr const LdrColorRGBA White(uint8 alpha = 255)    { return { LdrColorRGB::White(), alpha }; }
    static constexpr const LdrColorRGBA Silver(uint8 alpha = 255)   { return { LdrColorRGB::Silver(), alpha }; }
    static constexpr const LdrColorRGBA Gray(uint8 alpha = 255)     { return { LdrColorRGB::Gray(), alpha }; }
    static constexpr const LdrColorRGBA Black(uint8 alpha = 255)    { return { LdrColorRGB::Black(), alpha }; }
    static constexpr const LdrColorRGBA Red(uint8 alpha = 255)      { return { LdrColorRGB::Red(), alpha }; }
    static constexpr const LdrColorRGBA Maroon(uint8 alpha = 255)   { return { LdrColorRGB::Maroon(), alpha }; }
    static constexpr const LdrColorRGBA Lime(uint8 alpha = 255)     { return { LdrColorRGB::Lime(), alpha }; }
    static constexpr const LdrColorRGBA Green(uint8 alpha = 255)    { return { LdrColorRGB::Green(), alpha }; }
    static constexpr const LdrColorRGBA Blue(uint8 alpha = 255)     { return { LdrColorRGB::Blue(), alpha }; }
    static constexpr const LdrColorRGBA Navy(uint8 alpha = 255)     { return { LdrColorRGB::Navy(), alpha }; }
    static constexpr const LdrColorRGBA Yellow(uint8 alpha = 255)   { return { LdrColorRGB::Yellow(), alpha }; }
    static constexpr const LdrColorRGBA Olive(uint8 alpha = 255)    { return { LdrColorRGB::Olive(), alpha }; }
    static constexpr const LdrColorRGBA Cyan(uint8 alpha = 255)     { return { LdrColorRGB::Cyan(), alpha }; }
    static constexpr const LdrColorRGBA Teal(uint8 alpha = 255)     { return { LdrColorRGB::Teal(), alpha }; }
    static constexpr const LdrColorRGBA Magenta(uint8 alpha = 255)  { return { LdrColorRGB::Magenta(), alpha }; }
    static constexpr const LdrColorRGBA Purple(uint8 alpha = 255)   { return { LdrColorRGB::Purple(), alpha }; }
};

constexpr const LdrColorRGBA Lerp(const LdrColorRGBA colorA, const LdrColorRGBA colorB, uint8 factor)
{
    return
    {
        static_cast<uint8>((colorA.r * (uint32)(255 - factor) + colorB.r * (uint32)factor) / 256u),
        static_cast<uint8>((colorA.g * (uint32)(255 - factor) + colorB.g * (uint32)factor) / 256u),
        static_cast<uint8>((colorA.b * (uint32)(255 - factor) + colorB.b * (uint32)factor) / 256u),
        static_cast<uint8>((colorA.a * (uint32)(255 - factor) + colorB.a * (uint32)factor) / 256u),
    };
}

} // namespace Math
} // namespace NFE

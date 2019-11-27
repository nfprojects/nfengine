#pragma once

#include "../Raytracer.h"
#include "../Color/ColorHelpers.h"
#include "../../nfCommon/Math/Vector4.hpp"
#include "../../nfCommon/Math/HdrColor.hpp"
#include "../../nfCommon/Reflection/ReflectionClassMacros.hpp"
#include "../../nfCommon/Reflection/ReflectionEnumMacros.hpp"
#include "../../nfCommon/Reflection/Types/ReflectionDynArrayType.hpp"

namespace NFE {
namespace RT {

enum class Tonemapper : uint8
{
    Clamped,
    Reinhard,
    JimHejland_RichardBurgessDawson,
    ACES
};

class NFE_ALIGN(16) BloomElement
{
    NFE_DECLARE_CLASS(BloomElement);
public:

    float weight = 0.0f;
    float sigma = 5.0f;
    uint32 numBlurPasses = 8;
};

class NFE_ALIGN(16) BloomParams
{
    NFE_DECLARE_CLASS(BloomParams);
public:

    float factor; // bloom multiplier
    Common::DynArray<BloomElement> elements;

    NFE_RAYTRACER_API BloomParams();
};

class NFE_ALIGN(16) PostprocessParams
{
    NFE_DECLARE_CLASS(PostprocessParams);
public:

    Math::HdrColorRGB colorFilter;

    float exposure;             // exposure in log scale
    float contrast;
    float saturation;
    bool useDithering;
    BloomParams bloom;

    Tonemapper tonemapper;      // tonemapping curve

    NFE_RAYTRACER_API PostprocessParams();
};

template<typename T>
NFE_FORCE_INLINE static const T ToneMap(const T color, const Tonemapper tonemapper)
{
    T result;

    switch (tonemapper)
    {
    case Tonemapper::Clamped:
    {
        result = Convert_Linear_To_sRGB(color);
        break;
    }
    case Tonemapper::Reinhard:
    {
        result = Convert_Linear_To_sRGB(color / (T(1.0f) + color));
        break;
    }
    case Tonemapper::JimHejland_RichardBurgessDawson:
    {
        const T b = T(6.2f);
        const T c = T(1.7f);
        const T d = T(0.06f);
        const T t0 = color * T::MulAndAdd(color, b, T(0.5f));
        const T t1 = T::MulAndAdd(color, b, c);
        const T t2 = T::MulAndAdd(color, t1, d);
        result = t0 * T::FastReciprocal(t2);
        break;
    }
    case Tonemapper::ACES:
    {
        const T a = T(2.51f);
        const T b = T(0.03f);
        const T c = T(2.43f);
        const T d = T(0.59f);
        const T e = T(0.14f);
        const T t0 = color * T::MulAndAdd(color, a, b);
        const T t1 = T::MulAndAdd(color, c, d);
        const T t2 = T::MulAndAdd(color, t1, e);
        result = Convert_Linear_To_sRGB(t0 * T::FastReciprocal(t2));
        break;
    }
    default:
        NFE_FATAL("Invalid tonemapper");
    };

    return result;
}

} // namespace RT
} // namespace NFE

NFE_DECLARE_ENUM_TYPE(NFE::RT::Tonemapper);

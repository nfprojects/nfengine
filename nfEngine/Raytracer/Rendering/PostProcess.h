#pragma once

#include "../Raytracer.h"
#include "../Color/ColorHelpers.h"
#include "../../nfCommon/Math/Vector4.hpp"

namespace NFE {
namespace RT {

struct NFE_ALIGN(16) PostprocessParams
{
    Math::Vector4 colorFilter;

    float exposure;             // exposure in log scale
    float contrast;
    float saturation;
    float ditheringStrength;    // applied after tonemapping
    float bloomFactor;          // bloom multiplier

    // tonemapping curve
    Tonemapper tonemapper = Tonemapper::ACES;

    NFE_RAYTRACER_API PostprocessParams();


    NFE_RAYTRACER_API bool operator == (const PostprocessParams& other) const;
    NFE_RAYTRACER_API bool operator != (const PostprocessParams& other) const;
};

} // namespace RT
} // namespace NFE

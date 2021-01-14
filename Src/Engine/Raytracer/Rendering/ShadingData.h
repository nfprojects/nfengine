#pragma once

#include "../Raytracer.h"
#include "../Color/RayColor.h"
#include "../Traversal/Intersection.h"
#include "../../Common/Math/Matrix4.hpp"

namespace NFE {
namespace RT {

struct SampledMaterialParameters
{
    RayColor baseColor;
    RayColor emissionColor;
    float roughness;
    float roughnessAnisotropy;
    float metalness;
    float IoR;
};

struct ShadingData
{
    // geometry data
    IntersectionData intersection;

    // incoming ray data
    Math::Vec4f outgoingDirWorldSpace;

    SampledMaterialParameters materialParams;
};

} // namespace RT
} // namespace NFE

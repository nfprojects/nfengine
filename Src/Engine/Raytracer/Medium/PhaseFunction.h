#pragma once

#include "../RayLib.h"
#include "../Utils/Memory.h"
#include "../../Common/Math/Vec4f.hpp"

namespace NFE {
namespace RT {

// Henyey-Greenstein phase function
struct PhaseFunction
{
    static float Eval(const float cosTheta, const float g);
    static float Sample(const Math::Vec4f& outDir, Math::Vec4f& inDir, const float g, const Math::Vec2f& u);
};

} // namespace RT
} // namespace NFE

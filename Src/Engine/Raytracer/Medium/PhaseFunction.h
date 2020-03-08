#pragma once

#include "../RayLib.h"
#include "../Utils/Memory.h"
#include "../../Common/Math/Vector4.hpp"

namespace NFE {
namespace RT {

// Henyey-Greenstein phase function
struct PhaseFunction
{
    static float Eval(const float cosTheta, const float g);
    static float Sample(const Math::Vector4& outDir, Math::Vector4& inDir, const float g, const Math::Float2& u);
};

} // namespace RT
} // namespace NFE

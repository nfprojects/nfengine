#include "PCH.hpp"
#include "Vec4f.hpp"
#include "Utils.hpp"

namespace NFE {
namespace Math {


float FresnelDielectric(float NdV, float eta)
{
    if (NdV > 0.0f)
    {
        eta = 1.0f / eta;
    }

    const float c = fabsf(NdV);
    float g = eta * eta * (1.0f - NdV * NdV);

    if (g < 1.0f)
    {
        g = Sqrt(1.0f - g);
        const float A = (g - c) / (g + c);
        const float B = (c * (g + c) - 1.0f) / (c * (g - c) + 1.0f);
        return 0.5f * A * A * (1.0f + B * B);
    }

    // total internal reflection
    return 1.0f;
}

float FresnelMetal(const float NdV, const float eta, const float k)
{
    const float NdV2 = NdV * NdV;
    const float a = eta * eta + k * k;
    const float b = a * NdV2;
    const float rs = (b - (2.0f * eta * NdV) + 1.0f) / (b + (2.0f * eta * NdV) + 1.0f);
    const float rp = (a - (2.0f * eta * NdV) + NdV2) / (a + (2.0f * eta * NdV) + NdV2);
    return (rs + rp) * 0.5f;
}


} // namespace Math
} // namespace NFE

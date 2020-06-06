#include "PCH.hpp"
#include "Vec4f.hpp"

namespace NFE {
namespace Math {

const Vec4f Vec4f::Refract3(const Vec4f& i, const Vec4f& n, float eta)
{
    float NdotV = Vec4f::Dot3(i, n);
    if (NdotV < 0.0f)
    {
        eta = 1.0f / eta;
    }

    const float k = 1.0f - eta * eta * (1.0f - NdotV * NdotV);
    if (k <= 0.0f)
    {
        return Vec4f::Zero();
    }

    Vec4f transmitted = Vec4f::NegMulAndAdd(Vec4f(eta * NdotV + sqrtf(k)), n, i * eta);
    if (NdotV > 0.0f)
    {
        transmitted.z = -transmitted.z;
    }

    return transmitted.Normalized3();
}

} // namespace Math
} // namespace NFE

#pragma once

namespace NFE {
namespace Math {


// compute Fresnel reflection term for dielectric material
NFCOMMON_API float FresnelDielectric(float NdV, float eta);

// compute Fresnel reflection term for metalic material
NFCOMMON_API float FresnelMetal(const float NdV, const float eta, const float k);


} // namespace Math
} // namespace NFE

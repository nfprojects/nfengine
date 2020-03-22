#pragma once

#include "../nfCommon.hpp"

#include "Vector4.hpp"
#include "Vector2x8.hpp"
#include "Vector3x8.hpp"

namespace NFE {
namespace Math {

// Collection of helper functions to sample point on various 2D and 3D shapes.
// All the functions accepts 'u' parameter which is meant to be obtained from a sampler.
class SamplingHelpers
{
public:
    // get barycentric triangle coordinates
    NFCOMMON_API static const Vector4 GetTriangle(const Float2 u);

    // get point on a circle (radius = 1.0)
    NFCOMMON_API static const Vector4 GetCircle(const Float2 u);
    NFCOMMON_API static const Vector2x8 GetCircle_Simd8(const Vector2x8& u);

    // get point on a regular hexagon
    // Note: 3 sample components are required
    NFCOMMON_API static const Vector4 GetHexagon(const Float3 u);

    // TODO
    /*
    // get point on a regular polygon
    NFCOMMON_API static const Vector4 GetRegularPolygon(const uint32 n, const Vector4& u);
    NFCOMMON_API static const Vector2x8 GetRegularPolygon_Simd8(const uint32 n, const Vector2x8& u);
    */

    // get point on a sphere (radius = 1.0)
    NFCOMMON_API static const Vector4 GetSphere(const Float2 u);

    // get point on a hemisphere (uniform sampling, Z+ oriented)
    NFCOMMON_API static const Vector4 GetHemishpere(const Float2 u);

    // get point on a hemisphere with cosine distribution (0 at equator, 1 at pole)
    // typical usage is Lambertian BRDF sampling
    NFCOMMON_API static const Vector4 GetHemishpereCos(const Float2 u);


    // get float with normal (Gaussian) distribution
    // Note: 2 input values are required
    NFCOMMON_API static float GetFloatNormal(const Float2 u);

    // get 2D point with normal (Gaussian) distribution
    NFCOMMON_API static const Vector4 GetFloatNormal2(const Float2 u);
};


} // namespace Math
} // namespace NFE

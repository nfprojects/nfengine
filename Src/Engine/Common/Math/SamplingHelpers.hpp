#pragma once

#include "../nfCommon.hpp"

#include "Vec4f.hpp"
#include "Vec2x8f.hpp"
#include "Vec3x8f.hpp"

namespace NFE {
namespace Math {

// Collection of helper functions to sample point on various 2D and 3D shapes.
// All the functions accepts 'u' parameter which is meant to be obtained from a sampler.
class SamplingHelpers
{
public:
    // get barycentric triangle coordinates
    NFCOMMON_API static const Vec4f GetTriangle(const Vec2f u);

    // get point on a circle (radius = 1.0)
    NFCOMMON_API static const Vec4f GetCircle(const Vec2f u);
    NFCOMMON_API static const Vec2x8f GetCircle_Simd8(const Vec2x8f& u);

    // get point on a regular hexagon
    // Note: 3 sample components are required
    NFCOMMON_API static const Vec4f GetHexagon(const Vec3f u);

    // TODO
    /*
    // get point on a regular polygon
    NFCOMMON_API static const Vec4f GetRegularPolygon(const uint32 n, const Vec4f& u);
    NFCOMMON_API static const Vec2x8f GetRegularPolygon_Simd8(const uint32 n, const Vec2x8f& u);
    */

    // get point on a sphere (radius = 1.0)
    NFCOMMON_API static const Vec4f GetSphere(const Vec2f u);

    // get point on a hemisphere (uniform sampling, Z+ oriented)
    NFCOMMON_API static const Vec4f GetHemishpere(const Vec2f u);

    // get point on a hemisphere with cosine distribution (0 at equator, 1 at pole)
    // typical usage is Lambertian BRDF sampling
    NFCOMMON_API static const Vec4f GetHemishpereCos(const Vec2f u);


    // get float with normal (Gaussian) distribution
    // Note: 2 input values are required
    NFCOMMON_API static float GetFloatNormal(const Vec2f u);

    // get 2D point with normal (Gaussian) distribution
    NFCOMMON_API static const Vec4f GetFloatNormal2(const Vec2f u);
};


} // namespace Math
} // namespace NFE

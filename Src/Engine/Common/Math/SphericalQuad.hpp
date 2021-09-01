#pragma once

#include "Vec4f.hpp"

namespace NFE {
namespace Math {

// A helper structure for sampling a point on 3D quad
// that has uniform solid-angle distribution.
// Based on paper "An Area-Preserving Parametrization for Spherical Rectangles" by
// Carlos Urena, Marcos Fajardo and Alan King
struct SphericalQuad
{
    NFE_ALIGNED_CLASS(16)

    Vec4f o, x, y, z; // local reference system �R�
    float z0; //
    float x0, y0; // rectangle coords in �R�
    float x1, y1; //
    float b0, b1, k; // misc precomputed constants
    float S; // solid angle of �Q�

    NFE_INLINE void Init(const Vec4f& s, const Vec4f& ex, const Vec4f& ey, const Vec4f& o);
    NFE_INLINE float Pdf() const;
    NFE_INLINE const Vec4f Sample(float u, float v) const;
};

void SphericalQuad::Init(const Vec4f& s, const Vec4f& ex, const Vec4f& ey, const Vec4f& ref)
{
    o = ref;
    float exl = ex.Length3(), eyl = ey.Length3();
    // compute local reference system �R�
    x = ex / exl;
    y = ey / eyl;
    z = Vec4f::Cross3(x, y);
    // compute rectangle coords in local reference system
    Vec4f d = s - ref;
    z0 = Vec4f::Dot3(d, z);
    // flip �z� to make it point against �Q�
    if (z0 > 0.0f)
    {
        z *= -1.0f;
        z0 *= -1.0f;
    }
    x0 = Vec4f::Dot3(d, x);
    y0 = Vec4f::Dot3(d, y);
    x1 = x0 + exl;
    y1 = y0 + eyl;
    // create vectors to four vertices
    const Vec4f v00 = Vec4f(x0, y0, z0, 0.0f);
    const Vec4f v01 = Vec4f(x0, y1, z0, 0.0f);
    const Vec4f v10 = Vec4f(x1, y0, z0, 0.0f);
    const Vec4f v11 = Vec4f(x1, y1, z0, 0.0f);
    // compute normals to edges
    const Vec4f n0 = Vec4f::Cross3(v00, v10).Normalized3();
    const Vec4f n1 = Vec4f::Cross3(v10, v11).Normalized3();
    const Vec4f n2 = Vec4f::Cross3(v11, v01).Normalized3();
    const Vec4f n3 = Vec4f::Cross3(v01, v00).Normalized3();
    // compute internal angles (gamma_i)
    float g0 = acosf(-Vec4f::Dot3(n0, n1));
    float g1 = acosf(-Vec4f::Dot3(n1, n2));
    float g2 = acosf(-Vec4f::Dot3(n2, n3));
    float g3 = acosf(-Vec4f::Dot3(n3, n0));
    // compute predefined constants
    b0 = n0.z;
    b1 = n2.z;
    k = NFE_MATH_2PI - g2 - g3;
    // compute solid angle from internal angles
    S = g0 + g1 - k;
}

float SphericalQuad::Pdf() const
{
    return 1.0f / Max(NFE_MATH_EPSILON, S);
}

const Vec4f SphericalQuad::Sample(float u, float v) const
{
    // 1. compute �cu�
    float au = u * S + k;
    float fu = (cosf(au) * b0 - b1) / sinf(au);
    float cu = 1.0f / sqrtf(fu * fu + b0 * b0) * (fu > 0.0f ? 1.0f : -1.0f);
    cu = Clamp(cu, -1.0f, 1.0f); // avoid NaNs
    // 2. compute �xu�
    float xu = -(cu * z0) / sqrtf(1.0f - cu * cu);
    xu = Clamp(xu, x0, x1); // avoid Infs
    // 3. compute �yv�
    float d = sqrtf(xu * xu + z0 * z0);
    float h0 = y0 / sqrtf(d * d + y0 * y0);
    float h1 = y1 / sqrtf(d * d + y1 * y1);
    float hv = h0 + v * (h1 - h0);
    float hv2 = hv * hv;
    float yv = (hv2 < 1.0f - NFE_MATH_EPSILON) ? (hv * d) / sqrtf(1.0f - hv2) : y1;
    // 4. transform (xu,yv,z0) to world coords
    return o + xu * x + yv * y + z0 * z;
}

} // namespace Math
} // namespace NFE

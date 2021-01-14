#include "PCH.h"
#include "Microfacet.h"
#include "Sampling/GenericSampler.h"

namespace NFE {
namespace RT {

using namespace Math;

float Microfacet::HeightDistribution_P1(const float h)
{
    return NFE_MATH_INV_SQRT_2PI * expf(-0.5f * h * h);
}

float Microfacet::HeightDistribution_C1(const float h)
{
    return 0.5f + 0.5f * (float)Erf(NFE_MATH_INV_SQRT_2 * h);
}

float Microfacet::HeightDistribution_InvC1(const float U)
{
    return NFE_MATH_SQRT_2 * ErfInv(2.0f * U - 1.0f);
}

float Microfacet::D_wi(const Vec4f& wi, const Vec4f& wm) const
{
    if (wm.z <= 0.0f)
    {
        return 0.0f;
    }

    // normalization coefficient
    const float projectedarea = SlopeDistribution_ProjectedArea(wi);
    if (projectedarea == 0.0f)
    {
        return 0.0f;
    }

    const float c = 1.0f / projectedarea;

    return c * Max(0.0f, Vec4f::Dot3(wi, wm)) * D(wm);
}

float Microfacet::G1(const Vec4f& wi, const float h0) const
{
    if (wi.z > 0.9999f)
        return 1.0f;
    if (wi.z <= 0.0f)
        return 0.0f;

    // height CDF
    const float C1_h0 = HeightDistribution_C1(h0);
    // Lambda
    const float lambda = SlopeDistribution_Lambda(wi);
    // value
    return powf(C1_h0, lambda);
}

float Microfacet::SlopeDistribution_Lambda(const Vec4f& wi) const
{
    if (wi.z > 0.9999f)
    {
        return 0.0f;
    }

    if (wi.z < -0.9999f)
    {
        return -1.0f;
    }

    // a
    const float theta_i = acosf(wi.z);
    const float a = 1.0f / tanf(theta_i) / Alpha_i(wi);

    return 0.5f * (-1.0f + Signum(a) * sqrtf(1 + 1 / (a * a)));
}

float Microfacet::SlopeDistribution_ProjectedArea(const Vec4f& wi) const
{
    if (wi.z > 0.9999f)
        return 1.0f;
    if (wi.z < -0.9999f)
        return 0.0f;

    // a
    const float theta_i = acosf(wi.z);
    const float sin_theta_i = sinf(theta_i);

    const float alphai = Alpha_i(wi);

    // value
    const float value = 0.5f * (wi.z + sqrtf(wi.z * wi.z + sin_theta_i * sin_theta_i * alphai * alphai));

    return value;
}

float Microfacet::Alpha_i(const Vec4f& wi) const
{
    const float invSinTheta2 = 1.0f / (1.0f - wi.z * wi.z);
    const float cosPhi2 = Sqr(wi.x) * invSinTheta2;
    const float sinPhi2 = Sqr(wi.y) * invSinTheta2;
    return sqrtf(cosPhi2 * Sqr(mAlphaX) + sinPhi2 * Sqr(mAlphaY));
}

float Microfacet::SampleHeight(const Vec4f& wr, const float hr, const float u) const
{
    if (wr.z > 0.9999f)
    {
        return FLT_MAX;
    }

    if (wr.z < -0.9999f)
    {
        return HeightDistribution_InvC1(Clamp(u * HeightDistribution_C1(hr), 0.0001f, 0.9999f));
    }

    if (Abs(wr.z) < 0.0001f)
    {
        return hr;
    }

    // probability of intersection
    const float G_1 = G1(wr, hr);

    // leave the microsurface
    if (u > 1.0f - G_1)
    {
        return FLT_MAX;
    }

    // hit microsurface
    return HeightDistribution_InvC1(
        Clamp(
            HeightDistribution_C1(hr) / powf(1.0f - u, 1.0f / SlopeDistribution_Lambda(wr)
        ), 0.0001f, 0.9999f)
    );
}

float Microfacet::EvalPhaseFunction(const Vec4f& wi, const Vec4f& wo) const
{
    // half vector 
    const Vec4f wh = (wi + wo).Normalized3();
    if (wh.z < 0.0f)
    {
        return 0.0f;
    }

    // value
    return D_wi(wi, wh) / (4.0f * Vec4f::Dot3(wi, wh));
}

const Vec2f Microfacet::SampleP22_11(const float theta_i, const float U, const float U_2) const
{
    Vec2f slope;

    if (theta_i < 0.0001f)
    {
        const float r = Sqrt(U / (1.0f - U));
        const float phi = 6.28318530718f * U_2;
        slope.x = r * cosf(phi);
        slope.y = r * sinf(phi);
        return slope;
    }

    // constant
    const float sin_theta_i = sinf(theta_i);
    const float cos_theta_i = cosf(theta_i);
    const float tan_theta_i = sin_theta_i / cos_theta_i;

    // slope associated to theta_i
    const float slope_i = cos_theta_i / sin_theta_i;

    // projected area
    const float projectedarea = 0.5f * (cos_theta_i + 1.0f);
    if (projectedarea < 0.0001f || projectedarea != projectedarea)
    {
        return Vec2f(0.0f, 0.0f);
    }

    // normalization coefficient
    const float c = 1.0f / projectedarea;

    const float A = 2.0f * U / cos_theta_i / c - 1.0f;
    const float B = tan_theta_i;
    const float tmp = 1.0f / (A * A - 1.0f);

    const float D = Sqrt(Max(0.0f, B * B * tmp * tmp - (A * A - B * B) * tmp));
    const float slope_x_1 = B * tmp - D;
    const float slope_x_2 = B * tmp + D;
    slope.x = (A < 0.0f || slope_x_2 > 1.0f / tan_theta_i) ? slope_x_1 : slope_x_2;

    float U2;
    float S;
    if (U_2 > 0.5f)
    {
        S = 1.0f;
        U2 = 2.0f * (U_2 - 0.5f);
    }
    else
    {
        S = -1.0f;
        U2 = 2.0f * (0.5f - U_2);
    }

    const float z =
        (U2 * (U2 * (U2 * 0.27385f - 0.73369f) + 0.46341f)) /
        (U2 * (U2 * (U2 * 0.093073f + 0.309420f) - 1.000000f) + 0.597999f);

    slope.y = S * z * sqrtf(1.0f + slope.x * slope.x);

    return slope;
}

const Vec4f Microfacet::SampleD(const Vec4f& wi, const float U1, const float U2) const
{
    // stretch to match configuration with alpha=1.0	
    const Vec4f wi_11 = Vec4f(mAlphaX * wi.x, mAlphaY * wi.y, wi.z).Normalized3();

    // sample visible slope with alpha=1.0
    const Vec2f slope_11 = SampleP22_11(acosf(wi_11.z), U1, U2);

    // align with view direction
    const float phi = atan2(wi_11.y, wi_11.x);

    Vec2f slope
    {
        cosf(phi) * slope_11.x - sinf(phi) * slope_11.y,
        sinf(phi) * slope_11.x + cosf(phi) * slope_11.y
    };

    // stretch back
    slope.x *= mAlphaX;
    slope.y *= mAlphaY;

    // if numerical instability
    if ((slope.x != slope.x) || !isfinite(slope.x))
    {
        if (wi.z > 0.0f)
        {
            return Vec4f(0.0f, 0.0f, 1.0f);
        }
        else
        {
            return Vec4f(wi.x, wi.y, 0.0f).Normalized3();
        }
    }

    // compute normal
    return Vec4f(-slope.x, -slope.y, 1.0f).Normalized3();
}

} // namespace RT
} // namespace NFE

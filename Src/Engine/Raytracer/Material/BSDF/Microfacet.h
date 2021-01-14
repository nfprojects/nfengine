#pragma once

#include "../../Raytracer.h"
#include "../../../Common/Math/Transcendental.hpp"

namespace NFE {
namespace RT {

// TODO anisotropy

// GGX microfacet model
class Microfacet
{
public:
    NFE_FORCE_INLINE Microfacet(float alpha)
        : mAlpha(alpha)
        , mAlphaSqr(alpha * alpha)
        , mAlphaX(alpha)
        , mAlphaY(alpha)
    { }

    NFE_FORCE_INLINE Microfacet(float alpha, float anisotropy)
        : mAlpha(alpha)
        , mAlphaSqr(alpha * alpha)
        , mAlphaX(alpha * Math::Saturate(2.0f - 2.0f * anisotropy))
        , mAlphaY(alpha * Math::Saturate(2.0f * anisotropy))
    { }

    // distribution of normals (NDF)
    float D(const Math::Vec4f& m) const
    {
        const float NdotH = m.z;
        const float cosThetaSq = Math::Sqr(NdotH);
        const float tanThetaSq = Math::Max(1.0f - cosThetaSq, 0.0f) / cosThetaSq;
        const float cosThetaQu = cosThetaSq * cosThetaSq;
        return mAlphaSqr * NFE_MATH_INV_PI / (cosThetaQu * Math::Sqr(mAlphaSqr + tanThetaSq));
    }

    // distribution of visible normals (VNDF)
    float D_wi(const Math::Vec4f& wi, const Math::Vec4f& wm) const;

    // sample the VNDF
    const Math::Vec4f SampleD(const Math::Vec4f& wi, const float U1, const float U2) const;

    NFE_FORCE_INLINE float Pdf(const Math::Vec4f& m) const
    {
        return D(m) * Math::Abs(m.z);
    }

    // masking function
    float G1(const float NdotX) const
    {
        float tanThetaSq = Math::Max(1.0f - NdotX * NdotX, 0.0f) / (NdotX * NdotX);
        return 2.0f / (1.0f + Math::Sqrt(1.0f + mAlphaSqr * tanThetaSq));
    }

    // shadowing-masking term
    float G(const float NdotV, const float NdotL) const
    {
        float tanThetaSqV = (1.0f - NdotV * NdotV) / (NdotV * NdotV);
        float tanThetaSqL = (1.0f - NdotL * NdotL) / (NdotL * NdotL);
        return 4.0f / ((1.0f + Math::Sqrt(1.0f + mAlphaSqr * tanThetaSqV)) * (1.0f + Math::Sqrt(1.0f + mAlphaSqr * tanThetaSqL)));
    }

    // generate microfacet normal vector
    const Math::Vec4f Sample(const Math::Vec2f u) const
    {
        const float cosThetaSqr = (1.0f - u.x) / (1.0f + (mAlphaSqr - 1.0f) * u.x);
        const float cosTheta = Math::Sqrt(cosThetaSqr);
        const float sinTheta = Math::Sqrt(1.0f - cosThetaSqr);
        const float phi = NFE_MATH_2PI * u.y;
        const Math::Vec4f xy = sinTheta * Math::SinCos(phi);
        return Math::Vec4f::Select<0,0,1,0>(xy, Math::Vec4f(cosTheta));
    }

    // microsurface height PDF
    static float HeightDistribution_P1(const float h);

    // microsurface height CDF
    static float HeightDistribution_C1(const float h);

    // inverse of the microsurface height CDF
    static float HeightDistribution_InvC1(const float U);

    // masking function at height h0
    float G1(const Math::Vec4f& wi, const float h0) const;

    // sample microsurface height in outgoing direction
    float SampleHeight(const Math::Vec4f& wr, const float hr, const float U) const;

    // sample the distribution of visible slopes with alpha=1.0
    const Math::Vec2f SampleP22_11(const float theta_i, const float U1, const float U2) const;

    // evaluate local phase function 
    float EvalPhaseFunction(const Math::Vec4f& wi, const Math::Vec4f& wo) const;

    // projected roughness in wi
    float Alpha_i(const Math::Vec4f& wi) const;

    // projected area towards incident direction
    float SlopeDistribution_ProjectedArea(const Math::Vec4f& wi) const;

    // Smith's Lambda function
    float SlopeDistribution_Lambda(const Math::Vec4f& wi) const;

private:
    float mAlpha;
    float mAlphaSqr;

    float mAlphaX;
    float mAlphaY;
};

} // namespace RT
} // namespace NFE

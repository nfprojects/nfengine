#pragma once

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
    { }

    float D(const Math::Vector4& m) const
    {
        const float NdotH = m.z;
        const float cosThetaSq = Math::Sqr(NdotH);
        const float tanThetaSq = Math::Max(1.0f - cosThetaSq, 0.0f) / cosThetaSq;
        const float cosThetaQu = cosThetaSq * cosThetaSq;
        return mAlphaSqr * NFE_MATH_INV_PI / (cosThetaQu * Math::Sqr(mAlphaSqr + tanThetaSq));
    }

    NFE_FORCE_INLINE float Pdf(const Math::Vector4& m) const
    {
        return D(m) * Math::Abs(m.z);
    }

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

    const Math::Vector4 Sample(const Math::Float2 u) const
    {
        // generate microfacet normal vector using GGX distribution function (Trowbridge-Reitz)
        const float cosThetaSqr = (1.0f - u.x) / (1.0f + (mAlphaSqr - 1.0f) * u.x);
        const float cosTheta = Math::Sqrt(cosThetaSqr);
        const float sinTheta = Math::Sqrt(1.0f - cosThetaSqr);
        const float phi = NFE_MATH_2PI * u.y;
        const Math::Vector4 xy = sinTheta * Math::SinCos(phi);
        return Math::Vector4::Select<0,0,1,0>(xy, Math::Vector4(cosTheta));
    }

private:
    float mAlpha;
    float mAlphaSqr;
};

} // namespace RT
} // namespace NFE

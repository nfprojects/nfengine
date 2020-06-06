#pragma once

#include "BSDF.h"

namespace NFE {
namespace RT {

class RoughDiffuseBSDF : public BSDF
{
    NFE_DECLARE_POLYMORPHIC_CLASS(RoughDiffuseBSDF)

public:
    virtual const char* GetShortName() const override { return "roughDiffuse"; }
    virtual bool IsDelta() const override { return false; }
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const RayColor Evaluate(const EvaluationContext& ctx, float* outDirectPdfW = nullptr, float* outReversePdfW = nullptr) const override;
    virtual float Pdf(const EvaluationContext& ctx, PdfDirection dir) const override;

    static float Evaluate_Internal(const float NdotL, const float NdotV, const float LdotV, const float roughness);
};

} // namespace RT
} // namespace NFE

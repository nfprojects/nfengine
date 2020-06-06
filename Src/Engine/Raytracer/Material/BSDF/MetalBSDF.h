#pragma once

#include "BSDF.h"

namespace NFE {
namespace RT {

// Smooth metal (conductor) BRDF.
class MetalBSDF : public BSDF
{
    NFE_DECLARE_POLYMORPHIC_CLASS(MetalBSDF)

public:
    virtual const char* GetShortName() const override { return "metal"; }
    virtual bool IsDelta() const override { return true; }
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const RayColor Evaluate(const EvaluationContext& ctx, float* outDirectPdfW = nullptr, float* outReversePdfW = nullptr) const override;
    virtual float Pdf(const EvaluationContext& ctx, PdfDirection dir) const override;
};

} // namespace RT
} // namespace NFE

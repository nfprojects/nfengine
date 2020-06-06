#pragma once

#include "BSDF.h"

namespace NFE {
namespace RT {

// Rough metal (conductor) BRDF.
class RoughMetalBSDF : public BSDF
{
    NFE_DECLARE_POLYMORPHIC_CLASS(RoughMetalBSDF)

public:
    virtual const char* GetShortName() const override { return "roughMetal"; }
    virtual bool IsDelta() const override { return false; } // TODO depends on material
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const RayColor Evaluate(const EvaluationContext& ctx, float* outDirectPdfW = nullptr, float* outReversePdfW = nullptr) const override;
    virtual float Pdf(const EvaluationContext& ctx, PdfDirection dir) const override;
};

} // namespace RT
} // namespace NFE

#pragma once

#include "BSDF.h"

namespace NFE {
namespace RT {

// Smooth plastic-like BSDF
class PlasticBSDF : public BSDF
{
    NFE_DECLARE_POLYMORPHIC_CLASS(PlasticBSDF);

public:
    virtual const char* GetShortName() const override { return "plastic"; }
    virtual bool IsDelta() const override { return false; }
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const RayColor Evaluate(const EvaluationContext& ctx, float* outDirectPdfW = nullptr, float* outReversePdfW = nullptr) const override;
    virtual float Pdf(const EvaluationContext& ctx, PdfDirection dir) const override;
};

} // namespace RT
} // namespace NFE

#include "PCH.h"
#include "NullBSDF.h"

namespace NFE {
namespace RT {

const char* NullBSDF::GetName() const
{
    return "null";
}

bool NullBSDF::Sample(SamplingContext& ctx) const
{
    NFE_UNUSED(ctx);

    return false;
}

const RayColor NullBSDF::Evaluate(const EvaluationContext& ctx, float* outDirectPdfW, float* outReversePdfW) const
{
    NFE_UNUSED(ctx);
    NFE_UNUSED(outDirectPdfW);
    NFE_UNUSED(outReversePdfW);

    return RayColor::Zero();
}

float NullBSDF::Pdf(const EvaluationContext& ctx, PdfDirection dir) const
{
    NFE_UNUSED(ctx);
    NFE_UNUSED(dir);

    return 0.0f;
}

} // namespace RT
} // namespace NFE

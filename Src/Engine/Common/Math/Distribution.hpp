#pragma once

#include "../nfCommon.hpp"

namespace NFE {
namespace Math {

// Utility class for fast sampling 1D probability distribution function
// (piecewise constant)
class NFCOMMON_API Distribution
{
    NFE_MAKE_NONCOPYABLE(Distribution)
    NFE_MAKE_NONMOVEABLE(Distribution)

public:
    Distribution();
    ~Distribution();

    // initialize with 1D pdf function
    bool Initialize(const float* pdfValues, uint32 numValues);

    // sample discrete
    uint32 SampleDiscrete(const float u, float& outPdf) const;

private:
    float* mPDF;
    float* mCDF; // Cumulative distribution function
    uint32 mSize;
};

} // namespace Math
} // namespace NFE

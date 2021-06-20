#include "PCH.hpp"
#include "Distribution.hpp"
#include "Math.hpp"
#include "Memory/DefaultAllocator.hpp"
#include "Logger/Logger.hpp"


namespace NFE {
namespace Math {

Distribution::Distribution()
    : mPDF(nullptr)
    , mCDF(nullptr)
    , mSize(0)
{}

Distribution::~Distribution()
{
    NFE_FREE(mCDF);
    NFE_FREE(mPDF);

    mCDF = nullptr;
    mPDF = nullptr;
}

float Distribution::Pdf(uint32 valueIndex) const
{
    NFE_ASSERT(valueIndex < mSize, "Index out of bounds");
    return mPDF[valueIndex];
}

bool Distribution::Initialize(const float* pdfValues, uint32 numValues)
{
    if (numValues == 0)
    {
        NFE_LOG_ERROR("Empty distribution");
        return false;
    }

    if (!pdfValues)
    {
        NFE_LOG_ERROR("Invalid distribution pdf");
        return false;
    }

    mPDF = (float*)NFE_MALLOC(sizeof(float) * (size_t)numValues, NFE_CACHE_LINE_SIZE);
    if (!mPDF)
    {
        NFE_LOG_ERROR("Failed to allocate memory for PDF");
        return false;
    }

    mCDF = (float*)NFE_MALLOC(sizeof(float) * ((size_t)numValues + 1), NFE_CACHE_LINE_SIZE);
    if (!mCDF)
    {
        NFE_LOG_ERROR("Failed to allocate memory for CDF");
        return false;
    }

    // compute cumulated distribution function
    double accumulated = 0.0;
    mCDF[0] = 0.0f;
    for (uint32 i = 0; i < numValues; ++i)
    {
        NFE_ASSERT(IsValid(pdfValues[i]), "Corrupted pdf");
        NFE_ASSERT(pdfValues[i] >= 0.0f, "Pdf must be non-negative. Value number %u is %f", i, pdfValues[i]);

        accumulated += pdfValues[i];
        mCDF[i + 1] = static_cast<float>(accumulated);
    }

    NFE_ASSERT(accumulated > 0.0f, "Pdf must be non-zero");

    // normalize
    const float cdfNormFactor = 1.0f / static_cast<float>(accumulated);
    const float pdfNormFactor = cdfNormFactor * static_cast<float>(numValues);
    for (uint32 i = 0; i < numValues; ++i)
    {
        mCDF[i] *= cdfNormFactor;
        mPDF[i] = pdfValues[i] * pdfNormFactor;
    }
    mCDF[numValues] *= cdfNormFactor;

    // TODO Cumulative distribution function should be stored as unsigned integers, as it's only in 0-1 range

    mSize = numValues;
    return true;
}

uint32 Distribution::SampleDiscrete(const float u, float& outPdf) const
{
    uint32 low = 0u;
    uint32 high = mSize;

    // binary search
    while (low < high)
    {
        uint32 mid = (low + high) / 2u;
        if (u >= mCDF[mid])
        {
            low = mid + 1u;
        }
        else
        {
            high = mid;
        }
    }
    
    uint32 offset = low - 1u;

    NFE_ASSERT(offset < mSize, "");
    outPdf = mPDF[offset];

    return offset;
}

} // namespace Math
} // namespace NFE

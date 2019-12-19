#include "PCH.h"
#include "BitmapTexture.h"
#include "../Utils/Bitmap.h"
#include "../nfCommon/Math/ColorHelpers.hpp"
#include "../nfCommon/Logger/Logger.hpp"
#include "../nfCommon/Math/Distribution.hpp"
#include "../nfCommon/Containers/DynArray.hpp"

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

BitmapTexture::BitmapTexture() = default;
BitmapTexture::~BitmapTexture() = default;

BitmapTexture::BitmapTexture(const BitmapPtr& bitmap)
    : mBitmap(bitmap)
    , mFilter(BitmapTextureFilter::Linear_SmoothStep)
{}

const char* BitmapTexture::GetName() const
{
    if (!mBitmap)
    {
        return "<none>";
    }

    return mBitmap->GetDebugName();
}

const Vector4 BitmapTexture::Evaluate(const Vector4& coords) const
{
    const Bitmap* bitmapPtr = mBitmap.Get();

    if (!bitmapPtr)
    {
        return Vector4::Zero();
    }

    // bitmap size
    const VectorInt4 size = bitmapPtr->GetSize().Swizzle<0,1,0,1>();

    // wrap to 0..1 range
    const Vector4 warpedCoords = Vector4::Mod1(coords);

    // compute texel coordinates
    const Vector4 scaledCoords = warpedCoords * bitmapPtr->mFloatSize.Swizzle<0,1,0,1>();
    const VectorInt4 intCoords = VectorInt4::Convert(Vector4::Floor(scaledCoords));

    VectorInt4 texelCoords = intCoords;
    texelCoords -= VectorInt4::AndNot(intCoords < size, size);
    texelCoords += size & (intCoords < VectorInt4::Zero());

    Vector4 result;

    if (mFilter == BitmapTextureFilter::NearestNeighbor)
    {
        result = bitmapPtr->GetPixel(texelCoords.x, texelCoords.y);
    }
    else if (mFilter == BitmapTextureFilter::Linear || mFilter == BitmapTextureFilter::Linear_SmoothStep)
    {
        texelCoords = texelCoords.Swizzle<0, 1, 0, 1>();
        texelCoords += VectorInt4(0, 0, 1, 1);

        // wrap secondary coordinates
        texelCoords -= VectorInt4::AndNot(texelCoords < size, size);

        Vector4 colors[4];
        bitmapPtr->GetPixelBlock(texelCoords, colors);

        // bilinear interpolation
        Vector4 weights = scaledCoords - intCoords.ConvertToFloat();

        if (mFilter == BitmapTextureFilter::Linear_SmoothStep)
        {
            weights = SmoothStep(weights);
        }

        const Vector4 value0 = Vector4::Lerp(colors[0], colors[2], weights.SplatY());
        const Vector4 value1 = Vector4::Lerp(colors[1], colors[3], weights.SplatY());
        result = Vector4::Lerp(value0, value1, weights.SplatX());
    }
    else
    {
        NFE_FATAL("Invalid bitmap filter mode");
        result = Vector4::Zero();
    }

    NFE_ASSERT(result.IsValid());

    return result;
}

const Vector4 BitmapTexture::Sample(const Float2 u, Vector4& outCoords, float* outPdf) const
{
    NFE_ASSERT(mImportanceMap, "Bitmap texture is not samplable");

    float pdf = 0.0f;
    const uint32 pixelIndex = mImportanceMap->SampleDiscrete(u.x, pdf);

    const uint32 width = mBitmap->GetWidth();
    const uint32 height = mBitmap->GetHeight();

    // TODO get rid of division
    const uint32 x = pixelIndex % width;
    const uint32 y = pixelIndex / width;
    NFE_ASSERT(x < width);
    NFE_ASSERT(y < height);

    // TODO this is redundant, because BitmapTexture::Evaluate multiplies coords by size again...
    outCoords = Vector4::FromIntegers(x, y, 0, 0) / mBitmap->mFloatSize;

    if (outPdf)
    {
        *outPdf = pdf;
    }

    return BitmapTexture::Evaluate(outCoords);
}

bool BitmapTexture::MakeSamplable()
{
    if (mImportanceMap)
    {
        return true;
    }

    if (!mBitmap)
    {
        NFE_LOG_ERROR("BitmapTexture: Failed to build importance map, because bitmap is invalid");
        return false;
    }

    NFE_LOG_INFO("BitmapTexture: Generating importance map for bitmap '%s'...", mBitmap->GetDebugName());

    const uint32 width = mBitmap->GetWidth();
    const uint32 height = mBitmap->GetHeight();

    DynArray<float> importancePdf;
    importancePdf.Resize(width * height);

    for (uint32 j = 0; j < height; ++j)
    {
        for (uint32 i = 0; i < width; ++i)
        {
            const Vector4 value = mBitmap->GetPixel(i, j);
            importancePdf[width * j + i] = Vector4::Dot3(c_rgbIntensityWeights, value);
        }
    }

    mImportanceMap = MakeUniquePtr<Math::Distribution>();
    return mImportanceMap->Initialize(importancePdf.Data(), importancePdf.Size());
}

bool BitmapTexture::IsSamplable() const
{
    return mImportanceMap != nullptr;
}

} // namespace RT
} // namespace NFE

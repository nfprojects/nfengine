#include "PCH.h"
#include "BitmapTexture.h"
#include "../Utils/Bitmap.h"
#include "../Common/Math/ColorHelpers.hpp"
#include "../Common/Logger/Logger.hpp"
#include "../Common/Math/Distribution.hpp"
#include "../Common/Containers/DynArray.hpp"

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

const Vec4f BitmapTexture::Evaluate(const Vec4f& coords) const
{
    const Bitmap* bitmapPtr = mBitmap.Get();

    if (!bitmapPtr)
    {
        return Vec4f::Zero();
    }

    // bitmap size
    const Vec4i size = bitmapPtr->GetSize().Swizzle<0,1,0,1>();

    // wrap to 0..1 range
    const Vec4f warpedCoords = Vec4f::Mod1(coords);

    // compute texel coordinates
    const Vec4f scaledCoords = warpedCoords * bitmapPtr->mFloatSize.Swizzle<0,1,0,1>();
    const Vec4i intCoords = Vec4i::Convert(Vec4f::Floor(scaledCoords));

    Vec4i texelCoords = intCoords;
    texelCoords -= Vec4i::AndNot(intCoords < size, size);
    texelCoords += size & (intCoords < Vec4i::Zero());

    Vec4f result;

    if (mFilter == BitmapTextureFilter::NearestNeighbor)
    {
        result = bitmapPtr->GetPixel(texelCoords.x, texelCoords.y);
    }
    else if (mFilter == BitmapTextureFilter::Linear || mFilter == BitmapTextureFilter::Linear_SmoothStep)
    {
        texelCoords = texelCoords.Swizzle<0, 1, 0, 1>();
        texelCoords += Vec4i(0, 0, 1, 1);

        // wrap secondary coordinates
        texelCoords -= Vec4i::AndNot(texelCoords < size, size);

        Vec4f colors[4];
        bitmapPtr->GetPixelBlock(texelCoords, colors);

        // bilinear interpolation
        Vec4f weights = scaledCoords - intCoords.ConvertToVec4f();

        if (mFilter == BitmapTextureFilter::Linear_SmoothStep)
        {
            weights = SmoothStep(weights);
        }

        const Vec4f value0 = Vec4f::Lerp(colors[0], colors[2], weights.SplatY());
        const Vec4f value1 = Vec4f::Lerp(colors[1], colors[3], weights.SplatY());
        result = Vec4f::Lerp(value0, value1, weights.SplatX());
    }
    else
    {
        NFE_FATAL("Invalid bitmap filter mode");
        result = Vec4f::Zero();
    }

    NFE_ASSERT(result.IsValid());

    return result;
}

const Vec4f BitmapTexture::Sample(const Vec2f u, Vec4f& outCoords, float* outPdf) const
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
    outCoords = Vec4f::FromIntegers(x, y, 0, 0) / mBitmap->mFloatSize;

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
            const Vec4f value = mBitmap->GetPixel(i, j);
            importancePdf[width * j + i] = Vec4f::Dot3(c_rgbIntensityWeights, value);
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

#include "PCH.h"
#include "BitmapTexture3D.h"
#include "../Utils/Bitmap.h"
#include "../Common/Logger/Logger.hpp"

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

BitmapTexture3D::BitmapTexture3D() = default;
BitmapTexture3D::~BitmapTexture3D() = default;

BitmapTexture3D::BitmapTexture3D(const BitmapPtr& bitmap)
    : mBitmap(bitmap)
    , mFilter(BitmapTextureFilter::Linear)
{}

const char* BitmapTexture3D::GetName() const
{
    if (!mBitmap)
    {
        return "<none>";
    }

    return mBitmap->GetDebugName();
}

const Vec4f BitmapTexture3D::Evaluate(const Vec4f& coords) const
{
    const Bitmap* bitmapPtr = mBitmap.Get();

    if (!bitmapPtr)
    {
        return Vec4f::Zero();
    }

    // bitmap size
    const Vec4i size(bitmapPtr->GetSize());

    // wrap to 0..1 range
    const Vec4f warpedCoords = Vec4f::Mod1(coords * Vec4f(0.5f/1.25f, 0.5f/0.85f, 0.5f/1.53f) + Vec4f(0.5f, 0.5f, 0.5f));

    // compute texel coordinates
    const Vec4f scaledCoords = warpedCoords * bitmapPtr->mFloatSize;
    const Vec4i intCoords = Vec4i::TruncateAndConvert(scaledCoords);

    Vec4i texelCoords = intCoords;
    texelCoords -= Vec4i::AndNot(intCoords < size, size);
    texelCoords += size & (intCoords < Vec4i::Zero());

    Vec4f result;

    if (mFilter == BitmapTextureFilter::NearestNeighbor)
    {
        result = bitmapPtr->GetPixel3D(texelCoords.x, texelCoords.y, texelCoords.z);
    }
    else if (mFilter == BitmapTextureFilter::Linear)
    {
        Vec4i secondTexelCoords = texelCoords + Vec4i(1);

        // wrap secondary coordinates
        secondTexelCoords -= Vec4i::AndNot(secondTexelCoords < size, size);

        Vec4f colors[8];
        bitmapPtr->GetPixelBlock3D(Vec4ui(texelCoords), Vec4ui(secondTexelCoords), colors);

        // trilinear interpolation
        {
            const Vec4f weights = scaledCoords - intCoords.ConvertToVec4f();

            const Vec4f value00 = Vec4f::Lerp(colors[0], colors[1], weights.SplatX());
            const Vec4f value01 = Vec4f::Lerp(colors[2], colors[3], weights.SplatX());
            const Vec4f value10 = Vec4f::Lerp(colors[4], colors[5], weights.SplatX());
            const Vec4f value11 = Vec4f::Lerp(colors[6], colors[7], weights.SplatX());

            const Vec4f value0 = Vec4f::Lerp(value00, value01, weights.SplatY());
            const Vec4f value1 = Vec4f::Lerp(value10, value11, weights.SplatY());

            result = Vec4f::Lerp(value0, value1, weights.SplatZ());
        }
    }
    else
    {
        NFE_FATAL("Invalid bitmap filter mode");
        result = Vec4f::Zero();
    }

    NFE_ASSERT(result.IsValid());

    return result;
}

const Vec4f BitmapTexture3D::Sample(const Vec2f u, Vec4f& outCoords, float* outPdf) const
{
    // TODO

    NFE_FATAL("Bitmap texture is not samplable");

    NFE_UNUSED(u);
    NFE_UNUSED(outCoords);
    NFE_UNUSED(outPdf);

    return Vec4f::Zero();
}

bool BitmapTexture3D::MakeSamplable()
{
    // TODO

    return false;
}

bool BitmapTexture3D::IsSamplable() const
{
    return false;
}

} // namespace RT
} // namespace NFE

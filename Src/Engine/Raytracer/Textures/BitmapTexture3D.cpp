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

const Vector4 BitmapTexture3D::Evaluate(const Vector4& coords) const
{
    const Bitmap* bitmapPtr = mBitmap.Get();

    if (!bitmapPtr)
    {
        return Vector4::Zero();
    }

    // bitmap size
    const VectorInt4 size = bitmapPtr->GetSize();

    // wrap to 0..1 range
    const Vector4 warpedCoords = Vector4::Mod1(coords * Vector4(0.5f/1.25f, 0.5f/0.85f, 0.5f/1.53f) + Vector4(0.5f, 0.5f, 0.5f));

    // compute texel coordinates
    const Vector4 scaledCoords = warpedCoords * bitmapPtr->mFloatSize;
    const VectorInt4 intCoords = VectorInt4::TruncateAndConvert(scaledCoords);

    VectorInt4 texelCoords = intCoords;
    texelCoords -= VectorInt4::AndNot(intCoords < size, size);
    texelCoords += size & (intCoords < VectorInt4::Zero());

    Vector4 result;

    if (mFilter == BitmapTextureFilter::NearestNeighbor)
    {
        result = bitmapPtr->GetPixel3D(texelCoords.x, texelCoords.y, texelCoords.z);
    }
    else if (mFilter == BitmapTextureFilter::Linear)
    {
        VectorInt4 secondTexelCoords = texelCoords + VectorInt4(1);

        // wrap secondary coordinates
        secondTexelCoords -= VectorInt4::AndNot(secondTexelCoords < size, size);

        Vector4 colors[8];
        bitmapPtr->GetPixelBlock3D(texelCoords, secondTexelCoords, colors);

        // trilinear interpolation
        {
            const Vector4 weights = scaledCoords - intCoords.ConvertToFloat();

            const Vector4 value00 = Vector4::Lerp(colors[0], colors[1], weights.SplatX());
            const Vector4 value01 = Vector4::Lerp(colors[2], colors[3], weights.SplatX());
            const Vector4 value10 = Vector4::Lerp(colors[4], colors[5], weights.SplatX());
            const Vector4 value11 = Vector4::Lerp(colors[6], colors[7], weights.SplatX());

            const Vector4 value0 = Vector4::Lerp(value00, value01, weights.SplatY());
            const Vector4 value1 = Vector4::Lerp(value10, value11, weights.SplatY());

            result = Vector4::Lerp(value0, value1, weights.SplatZ());
        }
    }
    else
    {
        NFE_FATAL("Invalid bitmap filter mode");
        result = Vector4::Zero();
    }

    NFE_ASSERT(result.IsValid());

    return result;
}

const Vector4 BitmapTexture3D::Sample(const Float2 u, Vector4& outCoords, float* outPdf) const
{
    // TODO

    NFE_FATAL("Bitmap texture is not samplable");

    NFE_UNUSED(u);
    NFE_UNUSED(outCoords);
    NFE_UNUSED(outPdf);

    return Vector4::Zero();
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

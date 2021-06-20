#include "PCH.h"
#include "BitmapTexture.h"
#include "../Utils/Bitmap.h"
#include "../Common/Math/ColorHelpers.hpp"
#include "../Common/Math/Distribution.hpp"
#include "../Common/Math/WindowFunctions.hpp"
#include "../Common/Containers/DynArray.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"
#include "../Common/Utils/TaskBuilder.hpp"
#include "../Common/Utils/Waitable.hpp"

NFE_BEGIN_DEFINE_ENUM(NFE::RT::BitmapTextureFilter)
{
    NFE_ENUM_OPTION(NearestNeighbor)
    NFE_ENUM_OPTION(Linear)
    NFE_ENUM_OPTION(LinearSmoothStep)
    NFE_ENUM_OPTION(Bicubic)
}
NFE_END_DEFINE_ENUM()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::BitmapTexture)
{
    NFE_CLASS_PARENT(NFE::RT::ITexture);
    NFE_CLASS_MEMBER(mFilter);
    NFE_CLASS_MEMBER(mBicubicB).Norm();
    NFE_CLASS_MEMBER(mBicubicC).Norm();
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

BitmapTexture::BitmapTexture()
    : mFilter(BitmapTextureFilter::Linear)
    , mBicubicB(128)    // 0.50f
    , mBicubicC(64)     // 0.25f
{}

BitmapTexture::~BitmapTexture() = default;

BitmapTexture::BitmapTexture(const BitmapPtr& bitmap)
    : BitmapTexture()
{
    mBitmap = bitmap;
}


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
    const Vec4i size(bitmapPtr->GetSize().Swizzle<0,1,0,1>());

    // wrap to 0..1 range
    const Vec4f warpedCoords = Vec4f::Mod1(coords);

    // half pixel offset
    const Vec4f pixelOffset = mFilter != BitmapTextureFilter::NearestNeighbor ? Vec4f(0.5f) : Vec4f::Zero();

    // compute texel coordinates
    const Vec4f scaledCoords = warpedCoords * bitmapPtr->mFloatSize.Swizzle<0,1,0,1>() - pixelOffset;
    const Vec4i intCoords = Vec4i::Convert(Vec4f::Floor(scaledCoords));
    Vec4f coordFraction = scaledCoords - intCoords.ConvertToVec4f();

    Vec4i texelCoords = intCoords;
    texelCoords -= Vec4i::AndNot(intCoords < size, size);
    texelCoords += size & (intCoords < Vec4i::Zero());

    Vec4f result;

    if (mFilter == BitmapTextureFilter::NearestNeighbor)
    {
        result = bitmapPtr->GetPixel(texelCoords.x, texelCoords.y);
    }
    else if (mFilter == BitmapTextureFilter::Linear || mFilter == BitmapTextureFilter::LinearSmoothStep)
    {
        texelCoords = texelCoords.Swizzle<0, 1, 0, 1>();
        texelCoords += Vec4i(0, 0, 1, 1);

        // wrap secondary coordinates
        texelCoords -= Vec4i::AndNot(texelCoords < size, size);

        Vec4f colors[4];
        bitmapPtr->GetPixelBlock(Vec4ui(texelCoords), colors);

        if (mFilter == BitmapTextureFilter::LinearSmoothStep)
        {
            coordFraction = SmoothStep(coordFraction);
        }

        // bilinear interpolation
        const Vec4f value0 = Vec4f::Lerp(colors[0], colors[2], coordFraction.SplatY());
        const Vec4f value1 = Vec4f::Lerp(colors[1], colors[3], coordFraction.SplatY());
        result = Vec4f::Lerp(value0, value1, coordFraction.SplatX());
    }
    else if (mFilter == BitmapTextureFilter::Bicubic)
    {
        result = Vec4f::Zero();
        float weightSum = 0.0f;

        const float B = static_cast<float>(mBicubicB) / UINT8_MAX;
        const float C = static_cast<float>(mBicubicC) / UINT8_MAX;

        for (int32 j = -1; j <= 2; ++j)
        {
            for (int32 i = -1; i <= 2; ++i)
            {
                const Vec4i offset(i, j, 0, 0);
                Vec4i sampleCoords = intCoords + offset;
                
                // TODO some better wrapping
                sampleCoords -= Vec4i::AndNot(sampleCoords < size, size);
                sampleCoords -= Vec4i::AndNot(sampleCoords < size, size);
                sampleCoords += size & ((intCoords + offset) < Vec4i::Zero());

                const float d = (offset.ConvertToVec4f() - coordFraction).Length2();
                const float weight = WindowFunctions::MitchellNetravali(d, B, C);

                weightSum += weight;
                result += bitmapPtr->GetPixel(sampleCoords.x, sampleCoords.y) * weight;
                NFE_ASSERT(result.IsValid(), "");
            }
        }

        NFE_ASSERT(result.IsValid(), "");
        NFE_ASSERT(weightSum > 0.0f, "Invalid weight sum");
        result /= weightSum;
    }
    else
    {
        NFE_FATAL("Invalid bitmap filter mode");
        result = Vec4f::Zero();
    }

    NFE_ASSERT(result.IsValid(), "");

    return result;
}

float BitmapTexture::Pdf(SampleDistortion distortion, const Math::Vec4f& coords) const
{
    const Bitmap* bitmapPtr = mBitmap.Get();

    if (!bitmapPtr)
    {
        return 0.0f;
    }

    // bitmap size
    const Vec4i size(bitmapPtr->GetSize().Swizzle<0, 1, 0, 1>());

    // wrap to 0..1 range
    const Vec4f warpedCoords = Vec4f::Mod1(coords);

    // half pixel offset
    const Vec4f pixelOffset = mFilter != BitmapTextureFilter::NearestNeighbor ? Vec4f(0.5f) : Vec4f::Zero();

    // compute texel coordinates
    const Vec4f scaledCoords = warpedCoords * bitmapPtr->mFloatSize.Swizzle<0, 1, 0, 1>() - pixelOffset;
    const Vec4i intCoords = Vec4i::Convert(Vec4f::Floor(scaledCoords));

    Vec4i texelCoords = intCoords;
    texelCoords -= Vec4i::AndNot(intCoords < size, size);
    texelCoords += size & (intCoords < Vec4i::Zero());

    const uint32 valueIndex = texelCoords.x + texelCoords.y * bitmapPtr->GetWidth();

    return GetImportanceMap(distortion)->Pdf(valueIndex);
}

const Math::Distribution* BitmapTexture::GetImportanceMap(const SampleDistortion distortion) const
{
    if (distortion == SampleDistortion::Uniform)
    {
        return mImportanceMap[0].Get();
    }
    else if (distortion == SampleDistortion::Spherical)
    {
        return mImportanceMap[1].Get();
    }
    else
    {
        NFE_FATAL("Invalid distortion mode");
        return nullptr;
    }
}

const Vec4f BitmapTexture::Sample(const Vec3f u, Vec4f& outCoords, SampleDistortion distortion, float* outPdf) const
{
    const Math::Distribution* importanceMap = GetImportanceMap(distortion);
    NFE_ASSERT(importanceMap, "Bitmap texture is not samplable");

    float pdf = 0.0f;
    const uint32 pixelIndex = importanceMap->SampleDiscrete(u.x, pdf);

    const uint32 width = mBitmap->GetWidth();
    const uint32 height = mBitmap->GetHeight();

    // TODO get rid of division
    const uint32 x = pixelIndex % width;
    const uint32 y = pixelIndex / width;
    NFE_ASSERT(x < width, "");
    NFE_ASSERT(y < height, "");

    // TODO this is redundant, because BitmapTexture::Evaluate multiplies coords by size again...
    // TODO bilinar sampling?
    outCoords = (Vec4f::FromIntegers(x, y, 0, 0) + Vec4f(u.y, u.z)) / mBitmap->mFloatSize;

    if (outPdf)
    {
        *outPdf = pdf;
    }

    return BitmapTexture::Evaluate(outCoords);
}

bool BitmapTexture::MakeSamplable(SampleDistortion distortion)
{
    const Math::Distribution* importanceMap = GetImportanceMap(distortion);
    if (importanceMap)
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

    Waitable waitable;
    {
        TaskBuilder taskBuilder(waitable);
        taskBuilder.ParallelFor("BitmapTexture/MakeSamplable", height, [&](const TaskContext&, uint32 j)
        {
            float rowWeight = 1.0f;

            if (distortion == SampleDistortion::Spherical)
            {
                rowWeight = sinf(((float)j + 0.5f) * NFE_MATH_PI / (float)height);
            }

            for (uint32 i = 0; i < width; ++i)
            {
                const Vec4f value = mBitmap->GetPixel(i, j);

                NFE_ASSERT(value.IsValid(), "Invalid value in texture. X=%u, Y=%u", i, j);

                importancePdf[width * j + i] = Max(0.0f, rowWeight * Vec4f::Dot3(c_rgbIntensityWeights, value));
            }
        });
    }
    waitable.Wait();

    bool result = false;

    if (distortion == SampleDistortion::Uniform)
    {
        mImportanceMap[0] = MakeUniquePtr<Math::Distribution>();
        result =  mImportanceMap[0]->Initialize(importancePdf.Data(), importancePdf.Size());
    }
    else if (distortion == SampleDistortion::Spherical)
    {
        mImportanceMap[1] = MakeUniquePtr<Math::Distribution>();
        result = mImportanceMap[1]->Initialize(importancePdf.Data(), importancePdf.Size());
    }
    else
    {
        NFE_FATAL("Invalid distortion mode");
    }

    return result;
}

bool BitmapTexture::IsSamplable(SampleDistortion distortion) const
{
    return GetImportanceMap(distortion) != nullptr;
}

} // namespace RT
} // namespace NFE

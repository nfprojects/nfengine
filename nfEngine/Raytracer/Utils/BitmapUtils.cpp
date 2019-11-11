#include "PCH.h"
#include "BitmapUtils.h"
#include "../nfCommon/Math/Vector4Load.hpp"
#include "../nfCommon/Logger/Logger.hpp"
#include "../nfCommon/System/Assertion.hpp"
#include "../nfCommon/Utils/TaskBuilder.hpp"

#include "../nfCommon/Math/Vector8.hpp"


namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

// performs 1D box blur in linear time
template<typename T>
NFE_FORCE_NOINLINE
static void BoxBlur_Internal(T* __restrict targetLine, const T* __restrict srcLine, const uint32 radius, const uint32 width)
{
    const float factor = 1.0f / (float)(2u * radius + 1u);

    const T* __restrict srcLineBegin = srcLine;
    const T* __restrict srcLineEnd = srcLine;

    const T firstValue = srcLine[0];
    const T lastValue = srcLine[width - 1];
    T val = firstValue * static_cast<float>(radius + 1);

    for (uint32 j = 0; j < radius; j++)
    {
        val += *(srcLineBegin++);
    }

    for (uint32 j = 0; j <= radius; j++)
    {
        val += *(srcLineBegin++) - firstValue;
        *(targetLine++) = val * factor;
    }

    for (uint32 j = radius + 1; j < width - radius; j++)
    {
        val += *(srcLineBegin++) - *(srcLineEnd++);
        *(targetLine++) = val * factor;
    }

    for (uint32 j = width - radius; j < width; j++)
    {
        val += lastValue - *(srcLineEnd++);
        *(targetLine++) = val * factor;
    }
}


// Note: should be inside GaussianBlur, but there's bug in MSCV compiler and the const is not captured as const in a lambda...
static constexpr const uint32 MaxLineSize = 4096;
static constexpr const uint32 NumColumnsPerTask = 16;

using TempLineType = Vector4[NumColumnsPerTask][MaxLineSize];
static thread_local TempLineType gTempLineA;
static thread_local TempLineType gTempLineB;

bool BitmapUtils::GaussianBlur(Bitmap& targetBitmap, const Bitmap& sourceBitmap, const GaussianBlurParams params, Common::TaskBuilder& taskBuilder)
{
    NFE_ASSERT(params.numPasses > 0);

    if (targetBitmap.mFormat != Bitmap::Format::R32G32B32_Float)
    {
        NFE_LOG_ERROR("GaussianBlur: Unsupported texture format");
        return false;
    }

    // TODO get rid of this
    if (targetBitmap.GetWidth() > MaxLineSize || targetBitmap.GetHeight() > MaxLineSize)
    {
        NFE_LOG_ERROR("GaussianBlur: Image too big");
        return false;
    }

    if (targetBitmap.mFormat != sourceBitmap.mFormat)
    {
        NFE_LOG_ERROR("GaussianBlur: Source and target bitmap formats do not match");
        return false;
    }

    if ((targetBitmap.GetWidth() != sourceBitmap.GetWidth()) || (targetBitmap.GetHeight() != sourceBitmap.GetHeight()))
    {
        NFE_LOG_ERROR("GaussianBlur: Source and target bitmap dimensions do not match");
        return false;
    }

    // based on http://blog.ivank.net/fastest-gaussian-blur.html

    float wIdeal = sqrtf((12.0f * params.sigma * params.sigma / params.numPasses) + 1.0f);
    uint32 wl = (uint32)floorf(wIdeal);
    if (wl % 2 == 0)
    {
        wl--;
    }

    const uint32 wu = wl + 2;
    const float mIdeal = (12.0f * params.sigma * params.sigma - params.numPasses * wl * wl - 4.0f * params.numPasses * wl - 3.0f * params.numPasses) / (-4.0f * wl - 4.0f);
    const float m = roundf(mIdeal);

    const uint32 width = targetBitmap.GetWidth();
    const uint32 height = targetBitmap.GetHeight();

    // horizontal blur
    taskBuilder.ParallelFor("BitmapUtils::GaussianBlur/Horizontal", height, [=, &sourceBitmap, &targetBitmap] (const TaskContext&, const uint32 y)
    {
        Vector4* sourceLinePtr = gTempLineB[0];
        Vector4* targetLinePtr = gTempLineA[0];

        const Float3* sourceRowPtr = &sourceBitmap.GetPixelRef<Float3>(0, y);
        for (uint32 x = 0; x < width; ++x)
        {
            sourceLinePtr[x] = Vector4((float*)(sourceRowPtr + x));
        }

        for (uint32 i = 0; i < params.numPasses; ++i)
        {
            const uint32 radius = i < m ? wl : wu;
            BoxBlur_Internal(targetLinePtr, sourceLinePtr, radius, width);
            std::swap(sourceLinePtr, targetLinePtr);
        }

        Float3* targetRowPtr = &targetBitmap.GetPixelRef<Float3>(0, y);
        for (uint32 x = 0; x < width; ++x)
        {
            *(targetRowPtr + x) = sourceLinePtr[x].ToFloat3();
        }
    });

    taskBuilder.Fence();

    // vertical blur
    const uint32 numTasksForVerticalBlur = (width + NumColumnsPerTask - 1) / NumColumnsPerTask;
    taskBuilder.ParallelFor("BitmapUtils::GaussianBlur/Vertical", numTasksForVerticalBlur, [=, &sourceBitmap, &targetBitmap] (const TaskContext&, const uint32 columnGroupIndex)
    {
        // Note: in opossite to horizontal blur, vertical blur is done in batches of few columns to improve cache performance

        const uint32 numColumnsInTask = Min(width - columnGroupIndex * NumColumnsPerTask, NumColumnsPerTask);

        Vector4* sourceLinePtr = nullptr;
        Vector4* targetLinePtr = nullptr;

        for (uint32 y = 0; y < height; ++y)
        {
            const Float3* pixels = &targetBitmap.GetPixelRef<Float3>(columnGroupIndex * NumColumnsPerTask, y);
            for (uint32 i = 0; i < numColumnsInTask; ++i)
            {
                gTempLineA[i][y] = Vector4(pixels[i]);
            }
        }

        for (uint32 i = 0; i < numColumnsInTask; ++i)
        {
            sourceLinePtr = gTempLineA[i];
            targetLinePtr = gTempLineB[i];

            for (uint32 j = 0; j < params.numPasses; ++j)
            {
                const uint32 radius = j < m ? wl : wu;
                BoxBlur_Internal(targetLinePtr, sourceLinePtr, radius, height);
                std::swap(sourceLinePtr, targetLinePtr);
            }
        }

        const TempLineType& srcLine = params.numPasses % 2 == 0 ? gTempLineA : gTempLineB;

        for (uint32 y = 0; y < height; ++y)
        {
            Float3* pixels = &targetBitmap.GetPixelRef<Float3>(columnGroupIndex * NumColumnsPerTask, y);
            for (uint32 i = 0; i < numColumnsInTask; ++i)
            {
                pixels[i] = srcLine[i][y].ToFloat3();
            }
        }
    });

    return true;
}

} // namespace RT
} // namespace NFE

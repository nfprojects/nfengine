#include "PCH.h"
#include "Viewport.h"
#include "Film.h"
#include "Renderer.h"
#include "RendererContext.h"
#include "Tonemapping.h"
#include "Scene/Camera.h"
#include "Textures/Texture.h"
#include "Utils/BitmapUtils.h"
#include "Utils/Profiler.h"
#include "../Common/System/Timer.hpp"
#include "../Common/Math/SamplingHelpers.hpp"
#include "../Common/Math/PackedLoadVec4f.hpp"
#include "../Common/Math/Transcendental.hpp"
#include "../Common/Math/LdrColor.hpp"
#include "../Common/Math/ColorHelpers.hpp"
#include "../Common/Math/HilbertCurve.hpp"
#include "../Common/Logger/Logger.hpp"
#include "../Common/Utils/ThreadPool.hpp"
#include "../Common/Utils/Waitable.hpp"
#include "../Common/Reflection/Types/ReflectionClassType.hpp"
#include "../Common/Reflection/Types/ReflectionUniquePtrType.hpp"

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

static const uint32 MAX_IMAGE_SZIE = 1 << 16;

Viewport::Viewport()
    : mRenderer(nullptr)
{
    InitThreadData();

    mBlurredImages.Resize(mPostprocessParams.params.bloom.elements.Size());

    PrepareHilbertCurve(mParams.tileSize);
}

Viewport::~Viewport() = default;

void Viewport::InitThreadData()
{
    const uint32 numThreads = ThreadPool::GetInstance().GetNumThreads();

    mThreadData.Resize(numThreads);

    mSamplers.Clear();
    mSamplers.Reserve(numThreads);

    for (uint32 i = 0; i < numThreads; ++i)
    {
        RenderingContext& ctx = mThreadData[i];
        ctx.randomGenerator.Reset();
        ctx.sampler.fallbackGenerator = &ctx.randomGenerator;

        if (mRenderer)
        {
            ctx.rendererContext = mRenderer->CreateContext();
        }
    }
}

bool Viewport::InitBluredImages()
{
    Bitmap::InitData initData;
    initData.linearSpace = true;
    initData.width = GetWidth();
    initData.height = GetHeight();
    initData.format = Bitmap::Format::R32G32B32_Float;

    for (Bitmap& blurredImage : mBlurredImages)
    {
        if (!blurredImage.Init(initData))
        {
            return false;
        }
    }

    return true;
}

bool Viewport::Resize(uint32 width, uint32 height)
{
    if (width > MAX_IMAGE_SZIE || height > MAX_IMAGE_SZIE || width == 0 || height == 0)
    {
        NFE_LOG_ERROR("Invalid viewport size");
        return false;
    }

    if (width == GetWidth() && height == GetHeight())
    {
        return true;
    }

    Bitmap::InitData initData;
    initData.linearSpace = true;
    initData.width = width;
    initData.height = height;
    initData.format = Bitmap::Format::R32G32B32_Float;

    if (!mSum.Init(initData))
    {
        return false;
    }

    if (!mSecondarySum.Init(initData))
    {
        return false;
    }

    InitBluredImages();

    initData.linearSpace = false;
    initData.format = Bitmap::Format::B8G8R8A8_UNorm;
    if (!mFrontBuffer.Init(initData))
    {
        return false;
    }
    NFE_ASSERT(GetFrontBuffer().GetFormat() == Bitmap::Format::B8G8R8A8_UNorm);

    mPassesPerPixel.Resize(width * height);

    mPixelSalt.Resize(width * height);
    for (uint32 i = 0; i < width * height; ++i)
    {
        mPixelSalt[i] = mRandomGenerator.GetVec4f().ToVec2f();
    }

    Reset();

    return true;
}

void Viewport::SetPixelBreakpoint(uint32 x, uint32 y)
{
#ifndef NFE_CONFIGURATION_FINAL
    mPendingPixelBreakpoint.x = x;
    mPendingPixelBreakpoint.y = y;
#else
    NFE_UNUSED(x);
    NFE_UNUSED(y);
#endif
}

void Viewport::Reset()
{
    mPostprocessParams.fullUpdateRequired = true;

    mProgress = RenderingProgress();

    mHaltonSequence.Initialize(mParams.samplingParams.dimensions);

    mSum.Clear();
    mSecondarySum.Clear();

    memset(mPassesPerPixel.Data(), 0, sizeof(uint32) * GetWidth() * GetHeight());

    BuildInitialBlocksList();
}

bool Viewport::SetRenderer(IRenderer* renderer)
{
    mRenderer = renderer;

    InitThreadData();

    return true;
}

void Viewport::PrepareHilbertCurve(uint32 tileSize)
{
    tileSize = NextPowerOfTwo(tileSize);
    const uint32 numPixelsInTile = Sqr(tileSize);

    if (numPixelsInTile != mTileOffsets.Size())
    {
        mTileOffsets.Resize(numPixelsInTile);

        uint32 prevX = 0;
        uint32 prevY = 0;

        for (uint32 i = 0; i < numPixelsInTile; ++i)
        {
            uint32 x, y;
            Math::HilbertIndexToCoords(i, x, y);

            NFE_ASSERT(x < tileSize&& y < tileSize);
            mTileOffsets[i].x = static_cast<int8>(x - prevX);
            mTileOffsets[i].y = static_cast<int8>(y - prevY);

            prevX = x;
            prevY = y;
        }
    }
}

bool Viewport::SetRenderingParams(const RenderingParams& params)
{
    NFE_ASSERT(params.antiAliasingSpread >= 0.0f);
    NFE_ASSERT(params.motionBlurStrength >= 0.0f && params.motionBlurStrength <= 1.0f);

    mParams = params;

    PrepareHilbertCurve(mParams.tileSize);

    return true;
}

bool Viewport::SetPostprocessParams(const PostprocessParams& params)
{
    if (mBlurredImages.Size() != params.bloom.elements.Size())
    {
        mBlurredImages.Resize(params.bloom.elements.Size());
        InitBluredImages();
    }

    if (!RTTI::Compare(mPostprocessParams.params.lutParams, params.lutParams) ||
        !RTTI::Compare(mPostprocessParams.params.colorGradingParams, params.colorGradingParams) ||
        !RTTI::Compare(mPostprocessParams.params.tonemapper, params.tonemapper))
    {
        mPostprocessParams.lutGenerationRequired = true;
    }

    if (!RTTI::Compare(mPostprocessParams.params, params))
    {
        RTTI::GetType<PostprocessParams>()->Clone(&mPostprocessParams.params, &params);
        mPostprocessParams.fullUpdateRequired = true;
    }

    // TODO validation

    return true;
}

void Viewport::ComputeError()
{
    const Block fullImageBlock(0, GetWidth(), 0, GetHeight());
    mProgress.averageError = ComputeBlockError(fullImageBlock);
}

bool Viewport::Render(const Scene& scene, const Camera& camera)
{
    NFE_SCOPED_TIMER(Render);

    const uint32 width = GetWidth();
    const uint32 height = GetHeight();
    if (width == 0 || height == 0)
    {
        return false;
    }

    if (!mRenderer)
    {
        NFE_LOG_ERROR("Viewport: Missing renderer");
        return false;
    }

    DynArray<uint32> seed(mHaltonSequence.GetNumDimensions());
    {
        for (uint32 i = 0; i < mHaltonSequence.GetNumDimensions(); ++i)
        {
            seed[i] = mHaltonSequence.GetInt(i);
        }
        mHaltonSequence.NextSampleLeap();
    }

    Film film(mSum, mProgress.passesFinished % 2 == 0 ? &mSecondarySum : nullptr);
    const IRenderer::RenderParam renderParam = { scene, camera, mProgress.passesFinished, film };

    Waitable waitable;
    {
        TaskBuilder taskBuilder(waitable);

        for (uint32 i = 0; i < mThreadData.Size(); ++i)
        {
            RenderingContext& ctx = mThreadData[i];
            ctx.counters.Reset();
            ctx.params = &mParams;
            ctx.camera = &camera;
#ifndef NFE_CONFIGURATION_FINAL
            ctx.pixelBreakpoint = mPendingPixelBreakpoint;
#endif // NFE_CONFIGURATION_FINAL

            ctx.sampler.ResetFrame(seed, ctx.params->samplingParams.useBlueNoiseDithering);
        }

#ifndef NFE_CONFIGURATION_FINAL
        mPendingPixelBreakpoint.x = UINT32_MAX;
        mPendingPixelBreakpoint.y = UINT32_MAX;
#endif // NFE_CONFIGURATION_FINAL

        if (mRenderingTiles.Empty() || mProgress.passesFinished == 0)
        {
            GenerateRenderingTiles();
        }

        // randomize pixel offset (antialiasing)
        const Vec4f pixelOffset = SamplingHelpers::GetFloatNormal2(mRandomGenerator.GetVec2f());

        // pre-rendering pass
        mRenderer->PreRender(taskBuilder, renderParam, mThreadData);

        taskBuilder.Fence();

        // render tiles
        taskBuilder.ParallelFor("Render", mRenderingTiles.Size(), [pixelOffset, this, &renderParam] (const TaskContext& context, uint32 index)
        {
            const TileRenderingContext tileContext =
            {
                *mRenderer,
                renderParam,
                pixelOffset* mThreadData[0].params->antiAliasingSpread
            };
            RenderTile(tileContext, mThreadData[context.threadId], mRenderingTiles[index]);
        });

        taskBuilder.Fence();

        PerformPostProcess(taskBuilder);
    }
    waitable.Wait();

    mProgress.passesFinished++;

    if ((mProgress.passesFinished > 0) && (mProgress.passesFinished % 2 == 0))
    {
        if (mParams.adaptiveSettings.enable)
        {
            UpdateBlocksList();
            GenerateRenderingTiles();
        }
        else
        {
            ComputeError();
        }
    }

    // accumulate counters
    mCounters.Reset();
    for (const RenderingContext& ctx : mThreadData)
    {
        mCounters.Append(ctx.counters);
    }

    return true;
}

void Viewport::RenderTile(const TileRenderingContext& tileContext, RenderingContext& ctx, const Block& tile)
{
    NFE_SCOPED_TIMER(Viewport_RenderTile);

    Timer timer;

    NFE_ASSERT(tile.minX < tile.maxX);
    NFE_ASSERT(tile.minY < tile.maxY);
    NFE_ASSERT(tile.maxX <= GetWidth());
    NFE_ASSERT(tile.maxY <= GetHeight());

    const Vec4f filmSize = Vec4f::FromIntegers(GetWidth(), GetHeight(), 1, 1);
    const Vec4f invSize = VECTOR_ONE2 / filmSize;

    if (ctx.params->traversalMode == TraversalMode::Single)
    {
        uint32 x = tile.minX;
        uint32 y = tile.minY;

        for (const TileOffset& tileOffset : mTileOffsets)
        {
            x += tileOffset.x;
            y += tileOffset.y;

            if (x >= tile.maxX || y >= tile.maxY)
            {
                continue;
            }

            const uint32 realY = GetHeight() - 1u - y;

#ifndef NFE_CONFIGURATION_FINAL
            if (ctx.pixelBreakpoint.x == x && ctx.pixelBreakpoint.y == y)
            {
                NFE_BREAK();
            }
#endif // NFE_CONFIGURATION_FINAL

            const Vec4f coords = (Vec4f::FromIntegers(x, realY, 0, 0) + tileContext.sampleOffset) * invSize;

            ctx.sampler.ResetPixel(x, y);
            ctx.time = ctx.randomGenerator.GetFloat() * ctx.params->motionBlurStrength;
#ifdef NFE_ENABLE_SPECTRAL_RENDERING
            ctx.wavelength.Randomize(ctx.sampler.GetFloat());
#endif // NFE_ENABLE_SPECTRAL_RENDERING

            // generate primary ray
            const Ray ray = tileContext.renderParam.camera.GenerateRay(coords, ctx);

            if (ctx.params->visualizeTimePerPixel)
            {
                timer.Start();
            }

            RayColor color = tileContext.renderer.RenderPixel(ray, tileContext.renderParam, ctx);
            NFE_ASSERT(color.IsValid());

            if (ctx.params->visualizeTimePerPixel)
            {
                const float timePerRay = 1000.0f * static_cast<float>(timer.Stop());
                color = RayColor(timePerRay);
            }

            const Vec4f sampleColor = color.ConvertToTristimulus(ctx.wavelength);

#ifndef NFE_ENABLE_SPECTRAL_RENDERING
            // exception: in spectral rendering these values can get below zero due to RGB->Spectrum conversion
            NFE_ASSERT((sampleColor >= Vec4f::Zero()).All());
#endif // NFE_ENABLE_SPECTRAL_RENDERING

            tileContext.renderParam.film.AccumulateColor(x, y, sampleColor);
        }
    }
    else if (ctx.params->traversalMode == TraversalMode::Packet)
    {
        ctx.time = ctx.randomGenerator.GetFloat() * ctx.params->motionBlurStrength;
#ifdef NFE_ENABLE_SPECTRAL_RENDERING
        ctx.wavelength.Randomize(ctx.sampler.GetFloat());
#endif // NFE_ENABLE_SPECTRAL_RENDERING

        RayPacket& primaryPacket = ctx.rayPacket;
        primaryPacket.Clear();

        // TODO multisampling
        // TODO handle case where tile size does not fit ray group size

#if (NFE_RT_RAY_GROUP_SIZE == 4)

        NFE_ASSERT((tile.maxY - tile.minY) % 2 == 0);
        NFE_ASSERT((tile.maxX - tile.minX) % 2 == 0);

        constexpr uint32 rayGroupSizeX = 2;
        constexpr uint32 rayGroupSizeY = 2;

        for (uint32 y = tile.minY; y < tile.maxY; y += rayGroupSizeY)
        {
            const uint32 realY = GetHeight() - 1u - y;

            for (uint32 x = tile.minX; x < tile.maxX; x += rayGroupSizeX)
            {
                // generate ray group with following layout:
                //  0 1 2 3
                //  4 5 6 7
                Vec2x4f coords{ Vec4f::FromInteger(x), Vec4f::FromInteger(realY) };
                coords.x += Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
                coords.y -= Vec4f(0.0f, 0.0f, 1.0f, 1.0f);
                coords.x += Vec4f(tileContext.sampleOffset.x);
                coords.y += Vec4f(tileContext.sampleOffset.y);
                coords.x *= invSize.x;
                coords.y *= invSize.y;

                const ImageLocationInfo locations[] =
                {
                    { x + 0, y + 0 }, { x + 1, y + 0 }, { x + 0, y + 1 }, { x + 1, y + 1 },
                };

                const RayPacketTypes::Ray simdRay = tileContext.renderParam.camera.GenerateSimdRay(coords, ctx);
                primaryPacket.PushRays(simdRay, Vec3x4f(1.0f), locations);
            }
        }

#elif (NFE_RT_RAY_GROUP_SIZE == 8)

        NFE_ASSERT((tile.maxY - tile.minY) % 2 == 0);
        NFE_ASSERT((tile.maxX - tile.minX) % 4 == 0);

        constexpr uint32 rayGroupSizeX = 4;
        constexpr uint32 rayGroupSizeY = 2;

        for (uint32 y = tile.minY; y < tile.maxY; y += rayGroupSizeY)
        {
            const uint32 realY = GetHeight() - 1u - y;

            for (uint32 x = tile.minX; x < tile.maxX; x += rayGroupSizeX)
            {
                // generate ray group with following layout:
                //  0 1 2 3
                //  4 5 6 7
                Vec2x8f coords{ Vec8f::FromInteger(x), Vec8f::FromInteger(realY) };
                coords.x += Vec8f(0.0f, 1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 2.0f, 3.0f);
                coords.y -= Vec8f(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
                coords.x += Vec8f(tileContext.sampleOffset.x);
                coords.y += Vec8f(tileContext.sampleOffset.y);
                coords.x *= invSize.x;
                coords.y *= invSize.y;

                const ImageLocationInfo locations[] =
                {
                    { x + 0, y + 0 }, { x + 1, y + 0 }, { x + 2, y + 0 }, { x + 3, y + 0 },
                    { x + 0, y + 1 }, { x + 1, y + 1 }, { x + 2, y + 1 }, { x + 3, y + 1 },
                };

                const RayPacketTypes::Ray simdRay = tileContext.renderParam.camera.GenerateSimdRay(coords, ctx);
                primaryPacket.PushRays(simdRay, Vec3x8f(1.0f), locations);
            }
        }

#else
    #error Unsupported ray group size
#endif

        ctx.localCounters.Reset();
        tileContext.renderer.Raytrace_Packet(primaryPacket, tileContext.renderParam, ctx);
        ctx.counters.Append(ctx.localCounters);
    }

    ctx.counters.numPrimaryRays += (uint64)(tile.maxY - tile.minY) * (uint64)(tile.maxX - tile.minX);
}

void Viewport::PerformPostProcess(TaskBuilder& taskBuilder)
{
    NFE_SCOPED_TIMER(PerformPostProcess);

    if (!mBlurredImages.Empty() && mPostprocessParams.params.bloom.factor > 0.0f)
    {
        for (uint32 i = 0; i < mBlurredImages.Size(); ++i)
        {
            const Bitmap& sourceBitmap = i == 0 ? mSum : mBlurredImages[i - 1];

            BitmapUtils::GaussianBlurParams blurParams;
            blurParams.numPasses = mPostprocessParams.params.bloom.elements[i].numBlurPasses;
            blurParams.sigma = mPostprocessParams.params.bloom.elements[i].sigma;     
            BitmapUtils::GaussianBlur(mBlurredImages[i], sourceBitmap, blurParams, taskBuilder);

            taskBuilder.Fence();
        }
    }

    mPostprocessParams.colorScale = Vec4f(exp2f(mPostprocessParams.params.exposure));

    if (!mPostprocessLUT.IsGenerated() || mPostprocessParams.lutGenerationRequired)
    {
        mPostprocessLUT.Generate(mPostprocessParams.params);
        mPostprocessParams.lutGenerationRequired = false;
    }

    if (mPostprocessParams.fullUpdateRequired)
    {
        // post processing params has changed, perfrom full image update

        const uint32 numTiles = ThreadPool::GetInstance().GetNumThreads();

        const auto taskCallback = [this, numTiles] (const TaskContext& context, uint32 index)
        {
            Block block;
            block.minY = GetHeight() * index / numTiles;
            block.maxY = GetHeight() * (index + 1) / numTiles;
            block.minX = 0;
            block.maxX = GetWidth();

            PostProcessTile(block, context.threadId);
        };

        taskBuilder.ParallelFor("PostProcess_Full", numTiles, taskCallback);

        mPostprocessParams.fullUpdateRequired = false;
    }
    else
    {
        // apply post proces on active blocks only

        if (!mRenderingTiles.Empty())
        {
            const auto taskCallback = [this] (const TaskContext& context, uint32 index)
            {
                PostProcessTile(mRenderingTiles[index], context.threadId);
            };

            taskBuilder.ParallelFor("PostProcess", mRenderingTiles.Size(), taskCallback);
        }
    }
}

static void ApplyDither(Vec4f& color, Random& randomGenerator)
{
    // based on:
    // https://computergraphics.stackexchange.com/questions/5904/whats-a-proper-way-to-clamp-dither-noise/5952#5952
    // https://www.shadertoy.com/view/llXfzS

    // quantization scale (2^bits-1)
    const float scale = 255.0f;

    // TODO blue noise dithering
    const Vec4f u1 = randomGenerator.GetVec4f();
    const Vec4f u2 = randomGenerator.GetVec4f();

    // determine blending factor 't' for triangle/square noise
    const Vec4f lo = Vec4f::Min(Vec4f(1.0f), (2.0f * scale) * color);
    const Vec4f hi = Vec4f::NegMulAndAdd(color, 2.0f * scale, Vec4f(2.0f * scale));
    const Vec4f t = Vec4f::Min(lo, hi);

    // blend between triangle noise (middle range) and square noise (edges)
    // this is roughly equivalent to:
    //Vec4f ditherTri = u + v - Vec4f(1.0f);  // symmetric, triangular dither, [-1;1)
    //Vec4f ditherNorm = u - Vec4f(0.5f);     // symmetric, uniform dither [-0.5;0.5)
    //Vec4f dither = Vec4f::Lerp(ditherNorm, ditherTri, t) + Vec4f(0.5f);
    const Vec4f dither = u1 + t * (u2 - Vec4f(0.5f));

    // apply dither
    color += dither * (1.0f / scale);
}

void Viewport::PostProcessTile(const Block& block, uint32 threadID)
{
    NFE_SCOPED_TIMER(Viewport_PostProcessTile);

    Random& randomGenerator = mThreadData[threadID].randomGenerator;

    const PostprocessParams& params = mPostprocessParams.params;
    NFE_ASSERT(params.tonemapper, "Tonemapper missing");

    const bool useBloom = params.bloom.factor > 0.0f && !mBlurredImages.Empty();

    const float pixelScaling = 1.0f / (float)(1u + mProgress.passesFinished);
  
    for (uint32 y = block.minY; y < block.maxY; ++y)
    {
        for (uint32 x = block.minX; x < block.maxX; ++x)
        {
            const Vec4f rawValue = Vec4f_Load_Vec3f_Unsafe(mSum.GetPixelRef<Vec3f>(x, y));

#ifdef NFE_ENABLE_SPECTRAL_RENDERING
            Vec4f rgbColor;
            if (params.colorSpace == ColorSpace::Rec709)
            {
                rgbColor = ConvertXYZtoRec709(rawValue);
            }
            else if (params.colorSpace == ColorSpace::Rec2020)
            {
                rgbColor = ConvertXYZtoRec2020(rawValue);
            }
            else
            {
                NFE_FATAL("Invalid color space");
            }
            rgbColor = Vec4f::Max(Vec4f::Zero(), rgbColor);
#else
            Vec4f rgbColor = rawValue;
#endif

            // add bloom
            if (useBloom)
            {
                Vec4f bloomColor = Vec4f::Zero();
                for (uint32 i = 0; i < mBlurredImages.Size(); ++i)
                {
                    const Vec4f blurredColor = Vec4f_Load_Vec3f_Unsafe(mBlurredImages[i].GetPixelRef<Vec3f>(x, y));
                    bloomColor = Vec4f::MulAndAdd(blurredColor, params.bloom.elements[i].weight, bloomColor);
                }
                rgbColor = Vec4f::Lerp(rgbColor, bloomColor, params.bloom.factor);
            }

            // scale down by number of rendering passes finished
            // TODO support different number of passes per-pixel (adaptive rendering)
            rgbColor *= pixelScaling;

            // apply exposure
            rgbColor *= mPostprocessParams.colorScale;

            if (params.filmGrainStrength > 0.0f)
            {
                const float u = SamplingHelpers::GetFloatNormal(randomGenerator.GetVec2f());
                rgbColor *= FastExp2(params.filmGrainStrength * u);
            }

            // apply color grading & tonemapping
            rgbColor = mPostprocessLUT.Sample(rgbColor);

            // add dither
            if (params.useDithering)
            {
                ApplyDither(rgbColor, randomGenerator);
            }

            mFrontBuffer.GetPixelRef<uint32>(x, y) = rgbColor.ToBGR();
        }
    }
}

float Viewport::ComputeBlockError(const Block& block) const
{
    NFE_SCOPED_TIMER(ComputeBlockError);

    if (mProgress.passesFinished == 0)
    {
        return std::numeric_limits<float>::max();
    }

    NFE_ASSERT(mProgress.passesFinished % 2 == 0, "This funcion can be only called after even number of passes");

    const float imageScalingFactor = 1.0f / (float)mProgress.passesFinished;

    float totalError = 0.0f;
    for (uint32 y = block.minY; y < block.maxY; ++y)
    {
        float rowError = 0.0f;
        for (uint32 x = block.minX; x < block.maxX; ++x)
        {
            const Vec4f a = imageScalingFactor * Vec4f_Load_Vec3f_Unsafe(mSum.GetPixelRef<Vec3f>(x, y));
            const Vec4f b = (2.0f * imageScalingFactor) * Vec4f_Load_Vec3f_Unsafe(mSecondarySum.GetPixelRef<Vec3f>(x, y));
            const Vec4f diff = Vec4f::Abs(a - b);
            const float aLuminance = Vec4f::Dot3(c_rgbIntensityWeights, a);
            const float diffLuminance = Vec4f::Dot3(c_rgbIntensityWeights, diff);
            const float error = diffLuminance / Sqrt(NFE_MATH_EPSILON + aLuminance);
            rowError += error;
        }
        totalError += rowError;
    }

    const uint32 totalArea = GetWidth() * GetHeight();
    const uint32 blockArea = block.Width() * block.Height();
    return totalError * Sqrt((float)blockArea / (float)totalArea) / (float)blockArea;
}

void Viewport::GenerateRenderingTiles()
{
    NFE_SCOPED_TIMER(GenerateRenderingTiles);

    mRenderingTiles.Clear();
    mRenderingTiles.Reserve(mBlocks.Size());

    const uint32 tileSize = mParams.tileSize;

    for (const Block& block : mBlocks)
    {
        const uint32 rows = 1 + (block.Height() - 1) / tileSize;
        const uint32 columns = 1 + (block.Width() - 1) / tileSize;

        Block tile;

        for (uint32 j = 0; j < rows; ++j)
        {
            tile.minY = block.minY + j * tileSize;
            tile.maxY = Min(block.maxY, block.minY + j * tileSize + tileSize);
            NFE_ASSERT(tile.maxY > tile.minY);

            for (uint32 i = 0; i < columns; ++i)
            {
                tile.minX = block.minX + i * tileSize;
                tile.maxX = Min(block.maxX, block.minX + i * tileSize + tileSize);
                NFE_ASSERT(tile.maxX > tile.minX);

                mRenderingTiles.PushBack(tile);
            }
        }
    }
}

void Viewport::BuildInitialBlocksList()
{
    NFE_SCOPED_TIMER(BuildInitialBlocksList);

    mBlocks.Clear();

    const uint32 blockSize = mParams.adaptiveSettings.maxBlockSize;
    const uint32 rows = 1 + (GetHeight() - 1) / blockSize;
    const uint32 columns = 1 + (GetWidth() - 1) / blockSize;

    for (uint32 j = 0; j < rows; ++j)
    {
        Block block;

        block.minY = j * blockSize;
        block.maxY = Min(GetHeight(), (j + 1) * blockSize);
        NFE_ASSERT(block.maxY > block.minY);

        for (uint32 i = 0; i < columns; ++i)
        {
            block.minX = i * blockSize;
            block.maxX = Min(GetWidth(), (i + 1) * blockSize);
            NFE_ASSERT(block.maxX > block.minX);

            mBlocks.PushBack(block);
        }
    }

    mProgress.activeBlocks = mBlocks.Size();
}

void Viewport::UpdateBlocksList()
{
    NFE_SCOPED_TIMER(UpdateBlocksList);

    DynArray<Block> newBlocks;

    const AdaptiveRenderingSettings& settings = mParams.adaptiveSettings;

    if (mProgress.passesFinished < settings.numInitialPasses)
    {
        return;
    }

    for (uint32 i = 0; i < mBlocks.Size(); ++i)
    {
        const Block block = mBlocks[i];
        const float blockError = ComputeBlockError(block);

        if (blockError < settings.convergenceTreshold)
        {
            // block is fully converged - remove it
            mBlocks[i] = mBlocks.Back();
            mBlocks.PopBack();
            continue;
        }

        if ((blockError < settings.subdivisionTreshold) &&
            (block.Width() > settings.minBlockSize || block.Height() > settings.minBlockSize))
        {
            // block is somewhat converged - split it into two parts

            mBlocks[i] = mBlocks.Back();
            mBlocks.PopBack();

            Block childA, childB;

            // TODO split the block so the error is equal on both sides

            if (block.Width() > block.Height())
            {
                const uint32 halfPoint = (block.minX + block.maxX) / 2u;

                childA.minX = block.minX;
                childA.maxX = halfPoint;
                childA.minY = block.minY;
                childA.maxY = block.maxY;

                childB.minX = halfPoint;
                childB.maxX = block.maxX;
                childB.minY = block.minY;
                childB.maxY = block.maxY;
            }
            else
            {
                const uint32 halfPoint = (block.minY + block.maxY) / 2u;

                childA.minX = block.minX;
                childA.maxX = block.maxX;
                childA.minY = block.minY;
                childA.maxY = halfPoint;

                childB.minX = block.minX;
                childB.maxX = block.maxX;
                childB.minY = halfPoint;
                childB.maxY = block.maxY;
            }

            newBlocks.PushBack(childA);
            newBlocks.PushBack(childB);
        }
    }

    // add splitted blocks to the list
    mBlocks.Reserve(mBlocks.Size() + newBlocks.Size());
    for (const Block& block : newBlocks)
    {
        mBlocks.PushBack(block);
    }

    // calculate number of active pixels
    {
        mProgress.activePixels = 0;
        for (const Block& block : mBlocks)
        {
            mProgress.activePixels += block.Width() * block.Height();
        }
    }

    mProgress.converged = 1.0f - (float)mProgress.activePixels / (float)(GetWidth() * GetHeight());
    mProgress.activeBlocks = mBlocks.Size();
}

void Viewport::VisualizeActiveBlocks(Bitmap& bitmap) const
{
    NFE_SCOPED_TIMER(VisualizeActiveBlocks);

    const LdrColorRGBA color = LdrColorRGBA::Red();
    const uint8 alpha = 64;

    for (const Block& block : mBlocks)
    {
        for (uint32 y = block.minY; y < block.maxY; ++y)
        {
            for (uint32 x = block.minX; x < block.maxX; ++x)
            {
                LdrColorRGB& pixel = bitmap.GetPixelRef<LdrColorRGBA>(x, y);
                pixel = Lerp(pixel, color, alpha);
            }
        }

        for (uint32 y = block.minY; y < block.maxY; ++y)
        {
            bitmap.GetPixelRef<LdrColorRGB>(block.minX, y) = color;
        }

        for (uint32 y = block.minY; y < block.maxY; ++y)
        {
            bitmap.GetPixelRef<LdrColorRGB>(block.maxX - 1, y) = color;
        }

        for (uint32 x = block.minX; x < block.maxX; ++x)
        {
            bitmap.GetPixelRef<LdrColorRGB>(x, block.minY) = color;
        }

        for (uint32 x = block.minX; x < block.maxX; ++x)
        {
            bitmap.GetPixelRef<LdrColorRGB>(x, block.maxY - 1) = color;
        }
    }
}

} // namespace RT
} // namespace NFE

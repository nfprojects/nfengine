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
#include "../nfCommon/System/Timer.hpp"
#include "../nfCommon/Math/SamplingHelpers.hpp"
#include "../nfCommon/Math/Vector4Load.hpp"
#include "../nfCommon/Math/Transcendental.hpp"
#include "../nfCommon/Math/LdrColor.hpp"
#include "../nfCommon/Math/ColorHelpers.hpp"
#include "../nfCommon/Logger/Logger.hpp"
#include "../nfCommon/Utils/ThreadPool.hpp"
#include "../nfCommon/Utils/Waitable.hpp"
#include "../nfCommon/Reflection/Types/ReflectionClassType.hpp"

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
        mPixelSalt[i] = mRandomGenerator.GetVector4().ToFloat2();
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

bool Viewport::SetRenderingParams(const RenderingParams& params)
{
    NFE_ASSERT(params.antiAliasingSpread >= 0.0f);
    NFE_ASSERT(params.motionBlurStrength >= 0.0f && params.motionBlurStrength <= 1.0f);

    mParams = params;

    return true;
}

bool Viewport::SetPostprocessParams(const PostprocessParams& params)
{
    if (mBlurredImages.Size() != params.bloom.elements.Size())
    {
        mBlurredImages.Resize(params.bloom.elements.Size());
        InitBluredImages();
    }

    if (!RTTI::GetType<PostprocessParams>()->Compare(&mPostprocessParams.params, &params))
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

    mHaltonSequence.NextSample();
    DynArray<uint32> seed(mHaltonSequence.GetNumDimensions());
    for (uint32 i = 0; i < mHaltonSequence.GetNumDimensions(); ++i)
    {
        seed[i] = mHaltonSequence.GetInt(i);
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
        const Vector4 pixelOffset = SamplingHelpers::GetFloatNormal2(mRandomGenerator.GetFloat2());

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

    const Vector4 filmSize = Vector4::FromIntegers(GetWidth(), GetHeight(), 1, 1);
    const Vector4 invSize = VECTOR_ONE2 / filmSize;

    if (ctx.params->traversalMode == TraversalMode::Single)
    {
        for (uint32 y = tile.minY; y < tile.maxY; ++y)
        {
            const uint32 realY = GetHeight() - 1u - y;

            for (uint32 x = tile.minX; x < tile.maxX; ++x)
            {
#ifndef NFE_CONFIGURATION_FINAL
                if (ctx.pixelBreakpoint.x == x && ctx.pixelBreakpoint.y == y)
                {
                    NFE_BREAK();
                }
#endif // NFE_CONFIGURATION_FINAL

                const Vector4 coords = (Vector4::FromIntegers(x, realY, 0, 0) + tileContext.sampleOffset) * invSize;

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

                const Vector4 sampleColor = color.ConvertToTristimulus(ctx.wavelength);

#ifndef NFE_ENABLE_SPECTRAL_RENDERING
                // exception: in spectral rendering these values can get below zero due to RGB->Spectrum conversion
                NFE_ASSERT((sampleColor >= Vector4::Zero()).All());
#endif // NFE_ENABLE_SPECTRAL_RENDERING

                tileContext.renderParam.film.AccumulateColor(x, y, sampleColor);
            }
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
        NFE_ASSERT((tile.maxY - tile.minY) % 2 == 0);
        NFE_ASSERT((tile.maxX - tile.minX) % 4 == 0);
        /*
        for (uint32 y = tile.minY; y < tile.maxY; ++y)
        {
            const uint32 realY = GetHeight() - 1u - y;

            for (uint32 x = tile.minX; x < tile.maxX; ++x)
            {
                const Vector4 coords = (Vector4::FromIntegers(x, realY, 0, 0) + tileContext.sampleOffset) * invSize;

                for (uint32 s = 0; s < samplesPerPixel; ++s)
                {
                    // generate primary ray
                    const Ray ray = tileContext.camera.GenerateRay(coords, ctx);

                    const ImageLocationInfo location = { (uint16)x, (uint16)y };
                    primaryPacket.PushRay(ray, Vector4(sampleScale), location);
                }
            }
        }
        */

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
                Vector2x8 coords{ Vector8::FromInteger(x), Vector8::FromInteger(realY) };
                coords.x += Vector8(0.0f, 1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 2.0f, 3.0f);
                coords.y -= Vector8(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
                coords.x += Vector8(tileContext.sampleOffset.x);
                coords.y += Vector8(tileContext.sampleOffset.y);
                coords.x *= invSize.x;
                coords.y *= invSize.y;

                const ImageLocationInfo locations[] =
                {
                    { x + 0, y + 0 }, { x + 1, y + 0 }, { x + 2, y + 0 }, { x + 3, y + 0 },
                    { x + 0, y + 1 }, { x + 1, y + 1 }, { x + 2, y + 1 }, { x + 3, y + 1 },
                };

                const Ray_Simd8 simdRay = tileContext.renderParam.camera.GenerateRay_Simd8(coords, ctx);
                primaryPacket.PushRays(simdRay, Vector3x8(1.0f), locations);
            }
        }

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

    mPostprocessParams.colorScale = mPostprocessParams.params.colorFilter.ToVector4() * powf(2.0f, mPostprocessParams.params.exposure);

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

static void ApplyDither(Vector4& color, Random& randomGenerator)
{
    // based on:
    // https://computergraphics.stackexchange.com/questions/5904/whats-a-proper-way-to-clamp-dither-noise/5952#5952
    // https://www.shadertoy.com/view/llXfzS

    // quantization scale (2^bits-1)
    const float scale = 255.0f;

    // TODO blue noise dithering
    const Vector4 u1 = randomGenerator.GetVector4();
    const Vector4 u2 = randomGenerator.GetVector4();

    // determine blending factor 't' for triangle/square noise
    const Vector4 lo = Vector4::Min(Vector4(1.0f), (2.0f * scale) * color);
    const Vector4 hi = Vector4::NegMulAndAdd(color, 2.0f * scale, Vector4(2.0f * scale));
    const Vector4 t = Vector4::Min(lo, hi);

    // blend between triangle noise (middle range) and square noise (edges)
    // this is roughly equivalent to:
    //Vector4 ditherTri = u + v - Vector4(1.0f);  // symmetric, triangular dither, [-1;1)
    //Vector4 ditherNorm = u - Vector4(0.5f);     // symmetric, uniform dither [-0.5;0.5)
    //Vector4 dither = Vector4::Lerp(ditherNorm, ditherTri, t) + Vector4(0.5f);
    const Vector4 dither = u1 + t * (u2 - Vector4(0.5f));

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
  
    // TODO use AVX if available
    for (uint32 y = block.minY; y < block.maxY; ++y)
    {
        for (uint32 x = block.minX; x < block.maxX; ++x)
        {
            const Vector4 rawValue = Vector4_Load_Float3_Unsafe(mSum.GetPixelRef<Float3>(x, y));

#ifdef NFE_ENABLE_SPECTRAL_RENDERING
            Vector4 rgbColor;
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
            rgbColor = Vector4::Max(Vector4::Zero(), rgbColor);
#else
            Vector4 rgbColor = rawValue;
#endif

            // add bloom
            if (useBloom)
            {
                const float bloomFactor = params.bloom.factor;
                rgbColor *= 1.0f - bloomFactor;

                Vector4 bloomColor = Vector4::Zero();
                for (uint32 i = 0; i < mBlurredImages.Size(); ++i)
                {
                    const Vector4 blurredColor = Vector4_Load_Float3_Unsafe(mBlurredImages[i].GetPixelRef<Float3>(x, y));
                    bloomColor = Vector4::MulAndAdd(blurredColor, params.bloom.elements[i].weight, bloomColor);
                }
                rgbColor = Vector4::MulAndAdd(bloomColor, bloomFactor, rgbColor);
            }

            // scale down by number of rendering passes finished
            // TODO support different number of passes per-pixel (adaptive rendering)
            rgbColor *= pixelScaling;

            // apply saturation
            const float grayscale = Vector4::Dot3(rgbColor, c_rgbIntensityWeights);
            rgbColor = Vector4::Max(Vector4::Zero(), Vector4::Lerp(Vector4(grayscale), rgbColor, params.saturation));

            // apply contrast
            rgbColor = FastExp(FastLog(rgbColor) * params.contrast);

            // apply exposure
            rgbColor *= mPostprocessParams.colorScale;

            // apply tonemapping
            Vector4 toneMapped = params.tonemapper->Apply(rgbColor);

            // add dither
            if (params.useDithering)
            {
                ApplyDither(toneMapped, randomGenerator);
            }

            mFrontBuffer.GetPixelRef<uint32>(x, y) = toneMapped.ToBGR();
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
            const Vector4 a = imageScalingFactor * Vector4_Load_Float3_Unsafe(mSum.GetPixelRef<Float3>(x, y));
            const Vector4 b = (2.0f * imageScalingFactor) * Vector4_Load_Float3_Unsafe(mSecondarySum.GetPixelRef<Float3>(x, y));
            const Vector4 diff = Vector4::Abs(a - b);
            const float aLuminance = Vector4::Dot3(c_rgbIntensityWeights, a);
            const float diffLuminance = Vector4::Dot3(c_rgbIntensityWeights, diff);
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

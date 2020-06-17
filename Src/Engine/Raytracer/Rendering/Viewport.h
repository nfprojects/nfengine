#pragma once

#include "../Raytracer.h"

#include "RenderingParams.h"
#include "RenderingContext.h"
#include "Counters.h"
#include "PostProcess.h"
#include "Renderer.h"
#include "../Sampling/HaltonSampler.h"
#include "../Sampling/GenericSampler.h"
#include "../Utils/Bitmap.h"
#include "../Utils/Memory.h"
#include "../../Common/Math/Random.hpp"
#include "../../Common/Math/Rectangle.hpp"
#include "../../Common/Memory/Aligned.hpp"
#include "../../Common/Utils/TaskBuilder.hpp"

namespace NFE {
namespace RT {

using RendererPtr = Common::UniquePtr<IRenderer>;

struct RenderingProgress
{
    uint32 passesFinished = 0;
    uint32 activePixels = 0;
    uint32 activeBlocks = 0;
    float converged = 0.0f;
    float averageError = std::numeric_limits<float>::infinity();
};

class NFE_ALIGN(32) Viewport
{
    NFE_MAKE_NONCOPYABLE(Viewport)
    NFE_MAKE_NONMOVEABLE(Viewport)

public:
    NFE_ALIGNED_CLASS(32)

    NFE_RAYTRACER_API Viewport();
    NFE_RAYTRACER_API ~Viewport();

    NFE_RAYTRACER_API bool Resize(uint32 width, uint32 height);
    NFE_RAYTRACER_API bool SetRenderingParams(const RenderingParams& params);
    NFE_RAYTRACER_API bool SetRenderer(IRenderer* renderer);
    NFE_RAYTRACER_API bool SetPostprocessParams(const PostprocessParams& params);
    NFE_RAYTRACER_API bool Render(const Scene& scene, const Camera& camera);
    NFE_RAYTRACER_API void Reset();

    NFE_RAYTRACER_API void SetPixelBreakpoint(uint32 x, uint32 y);

    NFE_FORCE_INLINE const Bitmap& GetFrontBuffer() const { return mFrontBuffer; }
    NFE_FORCE_INLINE const Bitmap& GetSumBuffer() const { return mSum; }

    NFE_FORCE_INLINE uint32 GetWidth() const { return mSum.GetWidth(); }
    NFE_FORCE_INLINE uint32 GetHeight() const { return mSum.GetHeight(); }

    NFE_FORCE_INLINE const RenderingProgress& GetProgress() const { return mProgress; }
    NFE_FORCE_INLINE const RayTracingCounters& GetCounters() const { return mCounters; }

    NFE_RAYTRACER_API void VisualizeActiveBlocks(Bitmap& bitmap) const;

private:
    void InitThreadData();

    // region of a image used for adaptive rendering
    using Block = Math::Rectangle<uint32>;

    struct TileRenderingContext
    {
        const IRenderer& renderer;
        IRenderer::RenderParam renderParam;
        const Math::Vec4f sampleOffset;
    };

    struct NFE_ALIGN(16) PostprocessParamsInternal
    {
        PostprocessParams params;

        Math::Vec4f colorScale = Math::Vec4f::Zero();
        bool fullUpdateRequired = false;
        bool lutGenerationRequired = false;
    };

    struct TileOffset
    {
        int8 x : 4;
        int8 y : 4;
    };

    void BuildInitialBlocksList();

    // compute average error (variance) in the image
    void ComputeError();

    // calculate estimated error (variance) of a given block
    float ComputeBlockError(const Block& block) const;

    // generate list of tiles to be rendered (updates mRenderingTiles)
    void GenerateRenderingTiles();

    void UpdateBlocksList();

    // raytrace single image tile (will be called from multiple threads)
    void RenderTile(const TileRenderingContext& tileContext, RenderingContext& renderingContext, const Block& tile);

    bool InitBluredImages();
    void PerformPostProcess(Common::TaskBuilder& taskBuilder);

    // generate "front buffer" image from "sum" image
    void PostProcessTile(const Block& tile, uint32 threadID);

    void PrepareHilbertCurve(uint32 tileSize);

    IRenderer* mRenderer;

    Math::Random mRandomGenerator;
    HaltonSequence mHaltonSequence;

    Common::DynArray<GenericSampler> mSamplers;
    Common::DynArray<RenderingContext> mThreadData;

    Bitmap mSum;                        // image with accumulated samples (floating point, high dynamic range)
    Bitmap mSecondarySum;               // contains image with every second sample - required for adaptive rendering
    Bitmap mFrontBuffer;                // postprocesses image (low dynamic range)
    Common::DynArray<Bitmap> mBlurredImages;    // blurred images for bloom
    Common::DynArray<uint32> mPassesPerPixel;
    Common::DynArray<Math::Vec2f> mPixelSalt; // salt value for each pixel
    Common::DynArray<TileOffset> mTileOffsets;

    RenderingParams mParams;
    PostprocessParamsInternal mPostprocessParams;
    PostprocessLUT mPostprocessLUT;

    RayTracingCounters mCounters;

    RenderingProgress mProgress;

    Common::DynArray<Block> mBlocks;
    Common::DynArray<Block> mRenderingTiles;

#ifndef NFE_CONFIGURATION_FINAL
    PixelBreakpoint mPendingPixelBreakpoint;
#endif // NFE_CONFIGURATION_FINAL
};

} // namespace RT
} // namespace NFE

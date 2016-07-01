#pragma once

#include "../Raytracer.h"

#include "Context.h"
#include "Counters.h"
#include "PostProcess.h"
#include "Renderer.h"
#include "../Sampling/HaltonSampler.h"
#include "../Sampling/GenericSampler.h"
#include "../Utils/Bitmap.h"
#include "../Utils/Memory.h"
#include "../../nfCommon/Math/Random.hpp"
#include "../../nfCommon/Math/Rectangle.hpp"
#include "../../nfCommon/Memory/Aligned.hpp"
#include "../../nfCommon/Utils/TaskBuilder.hpp"

namespace NFE {
namespace RT {

using RendererPtr = Common::SharedPtr<IRenderer>;

struct RenderingProgress
{
    uint32 passesFinished = 0;
    uint32 activePixels = 0;
    uint32 activeBlocks = 0;
    float converged = 0.0f;
    float averageError = std::numeric_limits<float>::infinity();
};

class NFE_ALIGN(32) Viewport : public Common::Aligned<32>
{
    NFE_MAKE_NONCOPYABLE(Viewport);
    NFE_MAKE_NONMOVEABLE(Viewport);

public:
    NFE_RAYTRACER_API Viewport();
    NFE_RAYTRACER_API ~Viewport();

    NFE_RAYTRACER_API bool Resize(uint32 width, uint32 height);
    NFE_RAYTRACER_API bool SetRenderingParams(const RenderingParams& params);
    NFE_RAYTRACER_API bool SetRenderer(const RendererPtr& renderer);
    NFE_RAYTRACER_API bool SetPostprocessParams(const PostprocessParams& params);
    NFE_RAYTRACER_API bool Render(const Camera& camera);
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
        const Math::Vector4 sampleOffset;
    };

    struct NFE_ALIGN(16) PostprocessParamsInternal
    {
        PostprocessParams params;

        Math::Vector4 colorScale = Math::Vector4::Zero();
        bool fullUpdateRequired = false;
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

    void PerformPostProcess(Common::TaskBuilder& taskBuilder);

    // generate "front buffer" image from "sum" image
    void PostProcessTile(const Block& tile, uint32 threadID);

    RendererPtr mRenderer;

    Math::Random mRandomGenerator;
    HaltonSequence mHaltonSequence;

    Common::DynArray<GenericSampler> mSamplers;
    Common::DynArray<RenderingContext> mThreadData;

    Bitmap mSum;                        // image with accumulated samples (floating point, high dynamic range)
    Bitmap mSecondarySum;               // contains image with every second sample - required for adaptive rendering
    Bitmap mFrontBuffer;                // postprocesses image (low dynamic range)
    Common::DynArray<Bitmap> mBlurredImages;    // blurred images for bloom
    Common::DynArray<uint32> mPassesPerPixel;
    Common::DynArray<Math::Float2> mPixelSalt; // salt value for each pixel

    RenderingParams mParams;
    PostprocessParamsInternal mPostprocessParams;

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

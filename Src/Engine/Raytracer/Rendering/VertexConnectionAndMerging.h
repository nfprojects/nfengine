#pragma once

#include "Renderer.h"
#include "../Material/BSDF/BSDF.h"
#include "../../Common/Math/Packed.hpp"
#include "../../Common/Math/LdrColor.hpp"
#include "../../Common/Reflection/ReflectionEnumMacros.hpp"

// too slow for now...
//#define NFE_VCM_USE_KD_TREE

#ifdef NFE_VCM_USE_KD_TREE
#include "../Utils/KdTree.h"
#else
#include "../Utils/HashGrid.h"
#endif // NFE_VCM_USE_KD_TREE

namespace NFE {
namespace RT {

enum class VertexMergingKernel : uint8
{
    Uniform,
    Epanechnikov,
    Smooth,
};

// Vertex Connection and Merging
//
// Implements "Light Transport Simulation with Vertex Connection and Merging"
// Iliyan Georgiev, Jaroslav Krivanek, Tomas Davidovic, Philipp Slusallek
// ACM Transactions on Graphics 31(6) (SIGGRAPH Asia 2012).
//
// Based on https://github.com/SmallVCM/SmallVCM
//
class VertexConnectionAndMerging : public IRenderer
{
    NFE_DECLARE_POLYMORPHIC_CLASS(VertexConnectionAndMerging);

public:
    VertexConnectionAndMerging();
    ~VertexConnectionAndMerging();

    virtual RendererContextPtr CreateContext() const;

    virtual void PreRender(Common::TaskBuilder& builder, const RenderParam& renderParams, Common::ArrayView<RenderingContext> contexts) override;
    virtual const RayColor RenderPixel(const Math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const override;

    // for debugging
    Math::LdrColorRGB mBSDFSamplingWeight;
    Math::LdrColorRGB mLightSamplingWeight;
    Math::LdrColorRGB mVertexConnectingWeight;
    Math::LdrColorRGB mVertexMergingWeight;
    Math::LdrColorRGB mCameraConnectingWeight;

    uint32 mMaxPathLength;
    float mInitialMergingRadius;
    float mMinMergingRadius;
    float mMergingRadiusMultiplier;

    bool mUseVertexConnection;
    bool mUseVertexMerging;
    VertexMergingKernel mVertexMergingKernel;

    struct LightVertex
    {
        ShadingData shadingData;
        RayColor throughput;

        // quantities for MIS weight calculation
        float dVC;
        float dVM;
        float dVCM;

        uint8 pathLength;
    };

    struct NFE_ALIGN(32) Photon
    {
        Math::Vec3f position;
        Math::PackedColorRgbHdr throughput;
        Math::PackedUnitVector3 direction;

        // quantities for MIS weight calculation
        float dVM;  // TODO should be Half (watch out range)
        float dVCM; // TODO should be Half (watch out range)

        // used by hash grid query
        NFE_FORCE_INLINE const Math::Vec4f GetPosition() const
        {
            return *reinterpret_cast<const Math::Vec4f*>(this);
        }
    };

private:

    enum class PathType
    {
        Camera,
        Light,
    };

    // describes current state of path coming from camera or a light
    struct PathState
    {
        Math::Ray ray;
        RayColor throughput = RayColor::One();

        // quantities for MIS weight calculation
        float dVC = 0.0f;
        float dVM = 0.0f;
        float dVCM = 0.0f;

        uint32 length = 1u;
        BSDF::EventType lastSampledBsdfEvent = BSDF::NullEvent;
        bool lastSpecular = true;
        bool isFiniteLight = false;
    };

    // importance sample light sources
    const RayColor SampleLights(const Scene& scene, const ShadingData& shadingData, const PathState& pathState, RenderingContext& ctx) const;

    // importance sample single light source
    const RayColor SampleLight(const Scene& scene, const LightSceneObject* lightObject, const ShadingData& shadingData, const PathState& pathState, RenderingContext& ctx) const;

    // compute radiance from a hit local lights
    const RayColor EvaluateLight(uint32 iteration, const LightSceneObject* lightObject, const IntersectionData* intersection, const PathState& pathState, RenderingContext& ctx) const;

    // compute radiance from global lights
    const RayColor EvaluateGlobalLights(const Scene& scene, uint32 iteration, const PathState& pathState, RenderingContext& ctx) const;

    // generate initial camera ray
    bool GenerateCameraPath(PathState& path, RenderingContext& ctx) const;

    // generate initial light ray
    bool GenerateLightSample(const Scene& scene, PathState& pathState, RenderingContext& ctx) const;
    void TraceLightPath(const RenderParam& param, RenderingContext& ctx) const;

    // evaluate BSDF at ray's intersection and generate scattered ray
    bool AdvancePath(PathState& path, const ShadingData& shadingData, RenderingContext& ctx, PathType pathType) const;

    // connect a camera path end to a light path end and return contribution
    const RayColor ConnectVertices(const Scene& scene, PathState& cameraPathState, const ShadingData& shadingData, const LightVertex& lightVertex, RenderingContext& ctx) const;

    // merge a camera path vertex to light vertices nearby and return contribution
    const RayColor MergeVertices(PathState& cameraPathState, const ShadingData& shadingData, RenderingContext& ctx) const;

    // connect a light path to camera directly and splat the contribution onto film
    void ConnectToCamera(const RenderParam& renderParams, const LightVertex& lightVertex, RenderingContext& ctx) const;

    uint32 mLightPathsCount;

    float mMergingRadiusVC;
    float mMergingRadiusVM;
    float mInvSqrMergingRadiusVM;

    // computed based on merging radius and configuration
    float mVertexMergingNormalizationFactor;
    float mMisVertexMergingWeightFactorVC;
    float mMisVertexConnectionWeightFactorVC;
    float mMisVertexMergingWeightFactorVM;
    float mMisVertexConnectionWeightFactorVM;

    // acceleration structure used for vertex merging
#ifdef NFE_VCM_USE_KD_TREE
    KdTree mKdTree;
#else
    HashGrid mHashGrid;
#endif // NFE_VCM_USE_KD_TREE

    // list of all recorded light photons
    Common::DynArray<Photon> mPhotons;

    // summed counts of photons from each thread context
    Common::DynArray<uint32> mPhotonCountPrefixSum;
};

} // namespace RT
} // namespace NFE

NFE_DECLARE_ENUM_TYPE(NFE::RT::VertexMergingKernel);

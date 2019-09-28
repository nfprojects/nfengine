#pragma once

#include "Renderer.h"
#include "../Material/BSDF/BSDF.h"
#include "../../nfCommon/Math/Packed.hpp"

// too slow for now...
//#define NFE_VCM_USE_KD_TREE

#ifdef NFE_VCM_USE_KD_TREE
#include "../Utils/KdTree.h"
#else
#include "../Utils/HashGrid.h"
#endif // NFE_VCM_USE_KD_TREE

namespace NFE {
namespace RT {

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
public:
    VertexConnectionAndMerging(const Scene& scene);
    ~VertexConnectionAndMerging();

    virtual const char* GetName() const override;
    virtual RendererContextPtr CreateContext() const;

    virtual void PreRender(uint32 passNumber, const Film& film) override;
    virtual void PreRender(uint32 passNumber, RenderingContext& ctx) override;
    virtual void PreRenderGlobal(RenderingContext& ctx) override;
    virtual void PreRenderGlobal() override;
    virtual const RayColor RenderPixel(const Math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const override;

    // for debugging
    Math::Vector4 mBSDFSamplingWeight;
    Math::Vector4 mLightSamplingWeight;
    Math::Vector4 mVertexConnectingWeight;
    Math::Vector4 mVertexMergingWeight;
    Math::Vector4 mCameraConnectingWeight;

    uint32 mMaxPathLength;
    float mInitialMergingRadius;
    float mMinMergingRadius;
    float mMergingRadiusMultiplier;

    bool mUseVertexConnection;
    bool mUseVertexMerging;

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
        Math::Float3 position;
        Math::PackedColorRgbHdr throughput;
        Math::PackedUnitVector3 direction;

        // quantities for MIS weight calculation
        float dVM;  // TODO should be Half (watch out range)
        float dVCM; // TODO should be Half (watch out range)

        // used by hash grid query
        NFE_FORCE_INLINE const Math::Vector4 GetPosition() const
        {
            return *reinterpret_cast<const Math::Vector4*>(this);
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
    const RayColor SampleLights(const ShadingData& shadingData, const PathState& pathState, RenderingContext& ctx) const;

    // importance sample single light source
    const RayColor SampleLight(const LightSceneObject* lightObject, const ShadingData& shadingData, const PathState& pathState, RenderingContext& ctx) const;

    // compute radiance from a hit local lights
    const RayColor EvaluateLight(uint32 iteration, const LightSceneObject* lightObject, const IntersectionData* intersection, const PathState& pathState, RenderingContext& ctx) const;

    // compute radiance from global lights
    const RayColor EvaluateGlobalLights(uint32 iteration, const PathState& pathState, RenderingContext& ctx) const;

    // generate initial camera ray
    bool GenerateCameraPath(PathState& path, RenderingContext& ctx) const;

    // generate initial light ray
    bool GenerateLightSample(PathState& pathState, RenderingContext& ctx) const;
    void TraceLightPath(const Camera& camera, Film& film, RenderingContext& ctx) const;

    // evaluate BSDF at ray's intersection and generate scattered ray
    bool AdvancePath(PathState& path, const ShadingData& shadingData, RenderingContext& ctx, PathType pathType) const;

    // connect a camera path end to a light path end and return contribution
    const RayColor ConnectVertices(PathState& cameraPathState, const ShadingData& shadingData, const LightVertex& lightVertex, RenderingContext& ctx) const;

    // merge a camera path vertex to light vertices nearby and return contribution
    const RayColor MergeVertices(PathState& cameraPathState, const ShadingData& shadingData, RenderingContext& ctx) const;

    // connect a light path to camera directly and splat the contribution onto film
    void ConnectToCamera(const Camera& camera, Film& film, const LightVertex& lightVertex, RenderingContext& ctx) const;

    uint32 mLightPathsCount;

    float mMergingRadiusVC;
    float mMergingRadiusVM;

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
};

} // namespace RT
} // namespace NFE
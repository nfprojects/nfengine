#include "PCH.h"
#include "VertexConnectionAndMerging.h"
#include "RendererContext.h"
#include "RenderingContext.h"
#include "Film.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"
#include "Scene/Light/Light.h"
#include "Scene/Object/SceneObject_Light.h"
#include "Material/Material.h"
#include "Traversal/TraversalContext.h"
#include "Sampling/GenericSampler.h"
#include "Utils/Profiler.h"
#include "../nfCommon/Memory/MemoryHelpers.hpp"
#include "../nfCommon/Utils/TaskBuilder.hpp"
#include "../nfCommon/Reflection/ReflectionUtils.hpp"
#include "../nfCommon/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::VertexConnectionAndMerging)
{
    NFE_CLASS_PARENT(NFE::RT::IRenderer);
    NFE_CLASS_MEMBER(mBSDFSamplingWeight);
    NFE_CLASS_MEMBER(mLightSamplingWeight);
    NFE_CLASS_MEMBER(mVertexConnectingWeight);
    NFE_CLASS_MEMBER(mVertexMergingWeight);
    NFE_CLASS_MEMBER(mCameraConnectingWeight);
    NFE_CLASS_MEMBER(mMaxPathLength);
    NFE_CLASS_MEMBER(mInitialMergingRadius);
    NFE_CLASS_MEMBER(mMinMergingRadius);
    NFE_CLASS_MEMBER(mMergingRadiusMultiplier).Min(0.0f).Max(0.0f);
    NFE_CLASS_MEMBER(mUseVertexConnection);
    NFE_CLASS_MEMBER(mUseVertexMerging);
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

NFE_FORCE_INLINE static constexpr float Mis(const float samplePdf)
{
    return samplePdf;
}

NFE_FORCE_INLINE static constexpr float PdfWtoA(const float pdfW, const float distance, const float cosThere)
{
    return pdfW * Abs(cosThere) / Sqr(distance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static constexpr uint32 g_MaxLightVertices = 256;

class NFE_ALIGN(64) VertexConnectionAndMergingContext : public IRendererContext, public Aligned<64>
{
public:
    using Photon = VertexConnectionAndMerging::Photon;
    using LightVertex = VertexConnectionAndMerging::LightVertex;

    // list of photons recorded from a single thread
    DynArray<Photon> photons;

    // list of light vertices used in current pixel processing
    uint32 numLightVertices = 0;
    LightVertex lightVertices[g_MaxLightVertices];
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static_assert(sizeof(VertexConnectionAndMerging::Photon) == 32, "Invalid photon size");

VertexConnectionAndMerging::VertexConnectionAndMerging()
    : mLightPathsCount(0)
    , mBSDFSamplingWeight(LdrColorRGB::White())
    , mLightSamplingWeight(LdrColorRGB::White())
    , mVertexConnectingWeight(LdrColorRGB::White())
    , mCameraConnectingWeight(LdrColorRGB::White())
    , mVertexMergingWeight(LdrColorRGB::White())
{
    mUseVertexConnection = true;
    mUseVertexMerging = true;
    mMaxPathLength = 10;
    mInitialMergingRadius = 0.02f;
    mMergingRadiusVC = mMergingRadiusVM = mInitialMergingRadius;
    mMinMergingRadius = 0.02f;
    mMergingRadiusMultiplier = 1.0f; // 0.98f; // VCM_TODO
}

VertexConnectionAndMerging::~VertexConnectionAndMerging() = default;

RendererContextPtr VertexConnectionAndMerging::CreateContext() const
{
    return MakeUniquePtr<VertexConnectionAndMergingContext>();
}

/*
void VertexConnectionAndMerging::PreRender(uint32 passNumber, const Film& film)
{
    NFE_ASSERT(mInitialMergingRadius >= mMinMergingRadius);
    NFE_ASSERT(mMergingRadiusMultiplier > 0.0f);
    NFE_ASSERT(mMergingRadiusMultiplier <= 1.0f);
    NFE_ASSERT(mMaxPathLength > 0);

    mLightPathsCount = film.GetHeight() * film.GetWidth();

    if (passNumber == 0)
    {
        mMergingRadiusVC = mInitialMergingRadius;
        mMergingRadiusVM = mInitialMergingRadius;
    }
    else
    {
        // merging radius for vertex merging is delayed by 1 frame
        mMergingRadiusVM = mMergingRadiusVC;

        mMergingRadiusVC *= mMergingRadiusMultiplier;
        mMergingRadiusVC = Max(mMergingRadiusVC, mMinMergingRadius);
    }

    // Factor used to normalize vertex merging contribution.
    // We divide the summed up energy by disk radius and number of light paths
    mVertexMergingNormalizationFactor = 1.0f / (Sqr(mMergingRadiusVM) * NFE_MATH_PI * mLightPathsCount);

    // compute MIS weights for vertex connection
    {
        const float etaVCM = NFE_MATH_PI * Sqr(mMergingRadiusVC) * mLightPathsCount;
        // Note: we don't use merging in the first iteration
        mMisVertexMergingWeightFactorVC = (mUseVertexMerging && passNumber > 0) ? Mis(etaVCM) : 0.0f;
        mMisVertexConnectionWeightFactorVC = mUseVertexConnection ? Mis(1.f / etaVCM) : 0.0f;
    }

    // set MIS weights for vertex merging (delayed by 1 iteration)
    {
        const float etaVCM = NFE_MATH_PI * Sqr(mMergingRadiusVM) * mLightPathsCount;
        mMisVertexMergingWeightFactorVM = mUseVertexMerging ? Mis(etaVCM) : 0.0f;
        mMisVertexConnectionWeightFactorVM = mUseVertexConnection ? Mis(1.f / etaVCM) : 0.0f;
    }
}

void VertexConnectionAndMerging::PreRender(uint32 passNumber, RenderingContext& ctx)
{
    NFE_ASSERT(ctx.rendererContext);
    VertexConnectionAndMergingContext& rendererContext = *static_cast<VertexConnectionAndMergingContext*>(ctx.rendererContext.Get());

    if (passNumber == 0)
    {
        rendererContext.photons.Clear();
    }

    // TODO this is duplicated
    mPhotons.Clear();
}

void VertexConnectionAndMerging::PreRenderGlobal(RenderingContext& ctx)
{
    NFE_ASSERT(ctx.rendererContext);
    VertexConnectionAndMergingContext& rendererContext = *static_cast<VertexConnectionAndMergingContext*>(ctx.rendererContext.Get());

    NFE_SCOPED_TIMER(MergePhotonLists);

    // merge photon lists
    // TODO is there any way to get rid of this? (or at least make it multithreaded)
    const uint32 oldPhotonsSize = mPhotons.Size();
    const uint32 numPhotonsToAdd = rendererContext.photons.Size();
    mPhotons.Resize_SkipConstructor(oldPhotonsSize + numPhotonsToAdd);
    memcpy(mPhotons.Data() + oldPhotonsSize, rendererContext.photons.Data(), numPhotonsToAdd * sizeof(Photon));

    // prepare data structures
    rendererContext.photons.Clear();
}

void VertexConnectionAndMerging::PreRenderGlobal()
{
    // build hash grid of all light vertices
    // TODO make it multithreaded
    if (mUseVertexMerging)
    {
#ifdef NFE_VCM_USE_KD_TREE
        mKdTree.Build(mPhotons);
#else
        mHashGrid.Build(mPhotons, mMergingRadiusVM);
#endif // NFE_VCM_USE_KD_TREE
    }
}
*/

void VertexConnectionAndMerging::PreRender(Common::TaskBuilder& builder, const RenderParam& renderParams, Common::ArrayView<RenderingContext> contexts)
{
    NFE_ASSERT(mInitialMergingRadius >= mMinMergingRadius);
    NFE_ASSERT(mMergingRadiusMultiplier > 0.0f);
    NFE_ASSERT(mMergingRadiusMultiplier <= 1.0f);
    NFE_ASSERT(mMaxPathLength > 0);

    mLightPathsCount = renderParams.film.GetHeight() * renderParams.film.GetWidth();

    if (renderParams.iteration == 0)
    {
        mMergingRadiusVC = mInitialMergingRadius;
        mMergingRadiusVM = mInitialMergingRadius;
    }
    else
    {
        // merging radius for vertex merging is delayed by 1 frame
        mMergingRadiusVM = mMergingRadiusVC;

        mMergingRadiusVC *= mMergingRadiusMultiplier;
        mMergingRadiusVC = Max(mMergingRadiusVC, mMinMergingRadius);
    }

    // Factor used to normalize vertex merging contribution.
    // We divide the summed up energy by disk radius and number of light paths
    mVertexMergingNormalizationFactor = 1.0f / (Sqr(mMergingRadiusVM) * NFE_MATH_PI * mLightPathsCount);

    // compute MIS weights for vertex connection
    {
        const float etaVCM = NFE_MATH_PI * Sqr(mMergingRadiusVC) * mLightPathsCount;
        // Note: we don't use merging in the first iteration
        mMisVertexMergingWeightFactorVC = (mUseVertexMerging && renderParams.iteration > 0) ? Mis(etaVCM) : 0.0f;
        mMisVertexConnectionWeightFactorVC = mUseVertexConnection ? Mis(1.f / etaVCM) : 0.0f;
    }

    // set MIS weights for vertex merging (delayed by 1 iteration)
    {
        const float etaVCM = NFE_MATH_PI * Sqr(mMergingRadiusVM) * mLightPathsCount;
        mMisVertexMergingWeightFactorVM = mUseVertexMerging ? Mis(etaVCM) : 0.0f;
        mMisVertexConnectionWeightFactorVM = mUseVertexConnection ? Mis(1.f / etaVCM) : 0.0f;
    }

    // photon lists handling
    if (mUseVertexMerging)
    {
        mPhotonCountPrefixSum.Resize(contexts.Size());
        mPhotonCountPrefixSum[0] = 0;

        for (uint32 i = 0; i < contexts.Size(); ++i)
        {
            RenderingContext& ctx = contexts[i];
            VertexConnectionAndMergingContext& rendererContext = *static_cast<VertexConnectionAndMergingContext*>(ctx.rendererContext.Get());

            if (renderParams.iteration == 0)
            {
                rendererContext.photons.Clear();
            }

            // compute prefix sum
            if (i == 0)
            {
                mPhotonCountPrefixSum[i] = rendererContext.photons.Size();
            }
            else
            {
                mPhotonCountPrefixSum[i] = mPhotonCountPrefixSum[i - 1] + rendererContext.photons.Size();
            }
        }

        // prepare for merge
        mPhotons.Resize_SkipConstructor(mPhotonCountPrefixSum.Back());

        // merge photon lists from all thread contexts
        builder.ParallelFor("VCM/CopyPhotons", contexts.Size(), [this, contexts] (const TaskContext&, uint32 index)
        {
            RenderingContext& ctx = const_cast<RenderingContext&>(contexts[index]);
            VertexConnectionAndMergingContext& rendererContext = *static_cast<VertexConnectionAndMergingContext*>(ctx.rendererContext.Get());

            const uint32 numPhotonsToAdd = rendererContext.photons.Size();
            const uint32 offset = mPhotonCountPrefixSum[index] - numPhotonsToAdd;
            memcpy(mPhotons.Data() + offset, rendererContext.photons.Data(), numPhotonsToAdd * sizeof(Photon));

            rendererContext.photons.Clear();
        });

        // build acceleration structure of all photons vertices
        builder.Task("VCM/BuildAccelerationStructure", [this] (const TaskContext&)
        {
#ifdef NFE_VCM_USE_KD_TREE
            mKdTree.Build(mPhotons);
#else
            mHashGrid.Build(mPhotons, mMergingRadiusVM);
#endif // NFE_VCM_USE_KD_TREE
    });
}
}

const RayColor VertexConnectionAndMerging::RenderPixel(const Math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const
{
    // step 1: trace light paths & record photons

    TraceLightPath(param, ctx);


    // step 2: trace camera paths:

    const VertexConnectionAndMergingContext& rendererContext = *static_cast<const VertexConnectionAndMergingContext*>(ctx.rendererContext.Get());

    RayColor resultColor = RayColor::Zero();

    // initialize camera path
    PathState pathState{ ray };
    {
        const float cameraPdf = param.camera.PdfW(ray.dir);

        pathState.dVC = 0.0f;
        pathState.dVM = 0.0f;
        pathState.dVCM = Mis(1.0f / cameraPdf);
        pathState.lastSpecular = true;
    }

    HitPoint hitPoint;
    ShadingData shadingData;

    for (;;)
    {
        NFE_ASSERT(pathState.ray.IsValid());

        hitPoint.objectId = NFE_INVALID_OBJECT;
        hitPoint.distance = HitPoint::DefaultDistance;
        param.scene.Traverse({ pathState.ray, hitPoint, ctx });

        // ray missed - return background light color
        if (hitPoint.distance == HitPoint::DefaultDistance)
        {
            resultColor.MulAndAccumulate(pathState.throughput, EvaluateGlobalLights(param.scene, param.iteration, pathState, ctx));
            break;
        }

        // fill up structure with shading data
        param.scene.EvaluateIntersection(pathState.ray, hitPoint, ctx.time, shadingData.intersection);

        // update MIS quantities
        {
            const float cosTheta = Vector4::Dot3(pathState.ray.dir, shadingData.intersection.frame[2]);
            const float invMis = 1.0f / Mis(Abs(cosTheta));
            pathState.dVCM *= Mis(Sqr(hitPoint.distance));
            pathState.dVCM *= invMis;
            pathState.dVC *= invMis;
            pathState.dVM *= invMis;
        }

        // we hit a light directly
        if (hitPoint.subObjectId == NFE_LIGHT_OBJECT)
        {
            const ISceneObject* sceneObject = param.scene.GetHitObject(hitPoint.objectId);
            const LightSceneObject* lightObject = RTTI::Cast<LightSceneObject>(sceneObject);
            NFE_ASSERT(lightObject);

            const float cosAtLight = -shadingData.intersection.CosTheta(pathState.ray.dir);
            const RayColor lightColor = EvaluateLight(param.iteration, lightObject, &shadingData.intersection, pathState, ctx);
            NFE_ASSERT(lightColor.IsValid());
            resultColor.MulAndAccumulate(pathState.throughput, lightColor);
            break;
        }

        // fill up structure with shading data
        NFE_ASSERT(shadingData.intersection.material != nullptr);
        shadingData.outgoingDirWorldSpace = -pathState.ray.dir;
        shadingData.intersection.material->EvaluateShadingData(ctx.wavelength, shadingData);

        // accumulate material emission color
        // Note: no importance sampling for this
        {
            NFE_ASSERT(shadingData.materialParams.emissionColor.IsValid());
            resultColor.MulAndAccumulate(pathState.throughput, shadingData.materialParams.emissionColor);
        }

        if (pathState.length >= mMaxPathLength)
        {
            // path is too long for any other connection
            break;
        }

        const BSDF* bsdf = shadingData.intersection.material->GetBSDF();
        const bool isDeltaBsdf = bsdf && bsdf->IsDelta();

        // Vertex Connection -sample lights directly (a.k.a. next event estimation)
        if (!isDeltaBsdf && mUseVertexConnection)
        {
            const RayColor lightColor = SampleLights(param.scene, shadingData, pathState, ctx);
            NFE_ASSERT(lightColor.IsValid());
            resultColor.MulAndAccumulate(pathState.throughput, lightColor);
        }

        // Vertex Connection - connect camera vertex to light vertices (bidirectional path tracing)
        const uint32 numLightVertices = rendererContext.numLightVertices;
        if (!isDeltaBsdf && mUseVertexConnection && rendererContext.numLightVertices > 0)
        {
            RayColor vertexConnectionColor = RayColor::Zero();

            for (uint32 i = 0; i < numLightVertices; ++i)
            {
                const LightVertex& lightVertex = rendererContext.lightVertices[i];

                // full path would be too long
                // Note: all other light vertices can be skiped, as they will produce even longer paths
                if (lightVertex.pathLength + pathState.length + 1u > mMaxPathLength)
                {
                    break;
                }

                vertexConnectionColor.MulAndAccumulate(lightVertex.throughput, ConnectVertices(param.scene, pathState, shadingData, lightVertex, ctx));
            }

            vertexConnectionColor *= RayColor::ResolveRGB(ctx.wavelength, mVertexConnectingWeight);
            NFE_ASSERT(vertexConnectionColor.IsValid());
            resultColor.MulAndAccumulate(pathState.throughput, vertexConnectionColor);
        }

        // Vertex Merging - merge camera vertex to light vertices nearby
        if (!isDeltaBsdf && mUseVertexMerging && param.iteration > 0)
        {
            RayColor vertexMergingColor = MergeVertices(pathState, shadingData, ctx);
            NFE_ASSERT(vertexMergingColor.IsValid());
            vertexMergingColor *= RayColor::ResolveRGB(ctx.wavelength, mVertexMergingWeight);
            resultColor.MulAndAccumulate(pathState.throughput * vertexMergingColor, mVertexMergingNormalizationFactor);
        }

        // check if the ray depth won't be exeeded in the next iteration
        if (pathState.length > mMaxPathLength)
        {
            break;
        }

        // continue random walk
        if (!AdvancePath(pathState, shadingData, ctx, PathType::Camera))
        {
            break;
        }
    }

    // param.film.AccumulateColor(param.x, param.y, resultColor);
    return resultColor;
}

void VertexConnectionAndMerging::TraceLightPath(const RenderParam& param, RenderingContext& ctx) const
{
    NFE_ASSERT(ctx.rendererContext);
    VertexConnectionAndMergingContext& rendererContext = *static_cast<VertexConnectionAndMergingContext*>(ctx.rendererContext.Get());

    rendererContext.numLightVertices = 0;

    PathState pathState;

    if (!GenerateLightSample(param.scene, pathState, ctx))
    {
        // failed to generate initial ray - return empty path
        return;
    }

    HitPoint hitPoint;

    for (;;)
    {
        NFE_ASSERT(pathState.ray.IsValid());

        hitPoint.objectId = NFE_INVALID_OBJECT;
        hitPoint.distance = HitPoint::DefaultDistance;
        param.scene.Traverse({ pathState.ray, hitPoint, ctx });

        if (hitPoint.distance == HitPoint::DefaultDistance)
        {
            break; // ray missed
        }

        if (hitPoint.subObjectId == NFE_LIGHT_OBJECT)
        {
            break; // we hit a light directly
        }

        NFE_ASSERT(rendererContext.numLightVertices < g_MaxLightVertices);
        LightVertex& vertex = rendererContext.lightVertices[rendererContext.numLightVertices];

        // fill up structure with shading data
        ShadingData& shadingData = vertex.shadingData;
        {
            param.scene.EvaluateIntersection(pathState.ray, hitPoint, ctx.time, shadingData.intersection);

            shadingData.outgoingDirWorldSpace = -pathState.ray.dir;
            NFE_ASSERT(shadingData.intersection.material != nullptr);
            shadingData.intersection.material->EvaluateShadingData(ctx.wavelength, shadingData);
        }

        // update MIS quantities
        {
            if (pathState.length > 1 || pathState.isFiniteLight)
            {
                pathState.dVCM *= Mis(Sqr(hitPoint.distance));
            }

            const float cosTheta = Vector4::Dot3(pathState.ray.dir, shadingData.intersection.frame[2]);
            const float invMis = 1.0f / Mis(Abs(cosTheta));
            pathState.dVCM *= invMis;
            pathState.dVC  *= invMis;
            pathState.dVM  *= invMis;
        }

        // record the vertex for non-specular materials
        const BSDF* bsdf = shadingData.intersection.material->GetBSDF();
        if (bsdf && !bsdf->IsDelta())
        {
            // store light vertex for connection
            if (mUseVertexConnection)
            {
                rendererContext.numLightVertices++;

                vertex.pathLength = uint8(pathState.length);
                vertex.throughput = pathState.throughput;
                vertex.dVC = pathState.dVC;
                vertex.dVM = pathState.dVM;
                vertex.dVCM = pathState.dVCM;

                // connect vertex to camera directly
                ConnectToCamera(param, vertex, ctx);
            }

            // store simplified light vertex (photon) for merging
            if (mUseVertexMerging)
            {
                rendererContext.photons.EmplaceBack();
                Photon& photon = rendererContext.photons.Back();

                photon.position = shadingData.intersection.frame[3].ToFloat3();
                photon.direction.FromVector(shadingData.outgoingDirWorldSpace);
                photon.throughput.FromVector(pathState.throughput.ConvertToTristimulus(ctx.wavelength));
                photon.dVM = pathState.dVM;
                photon.dVCM = pathState.dVCM;
                //photon.pathLength = uint8(pathState.length);
            }
        }

        if (pathState.length + 2 > mMaxPathLength)
        {
            break; // check if the ray depth won't be exeeded in the next iteration
        }

        if (!AdvancePath(pathState, shadingData, ctx, PathType::Light))
        {
            break;
        }
    }
}

bool VertexConnectionAndMerging::GenerateLightSample(const Scene& scene, PathState& outPath, RenderingContext& ctx) const
{
    const auto& allLocalLights = scene.GetLights();
    if (allLocalLights.Empty())
    {
        // no lights on the scene
        return false;
    }

    const float lightPickProbability = 1.0f / (float)allLocalLights.Size();
    const uint32 lightIndex = ctx.randomGenerator.GetInt() % allLocalLights.Size(); // TODO get rid of division

    const LightSceneObject* lightObject = allLocalLights[lightIndex];
    const ILight& light = lightObject->GetLight();

    const ILight::EmitParam emitParam =
    {
        lightObject->GetTransform(ctx.time),
        ctx.wavelength,
        ctx.randomGenerator.GetFloat3(),
        ctx.randomGenerator.GetFloat2(),
    };

    ILight::EmitResult emitResult;
    const RayColor throughput = light.Emit(emitParam, emitResult);

    if (throughput.AlmostZero())
    {
        // generated sample is too weak - skip it
        return false;
    }

    NFE_ASSERT(emitResult.emissionPdfW > 0.0f);

    emitResult.directPdfA *= lightPickProbability;
    emitResult.emissionPdfW *= lightPickProbability;
    
    const float emissionInvPdfW = 1.0f / emitResult.emissionPdfW;

    emitResult.position += emitResult.direction * 0.0005f;
    outPath.ray = Ray(emitResult.position, emitResult.direction);
    outPath.throughput = throughput * emissionInvPdfW;

    const ILight::Flags lightFlags = light.GetFlags();
    outPath.isFiniteLight = lightFlags & ILight::Flag_IsFinite;

    // setup MIS weights
    {
        outPath.dVCM = Mis(emitResult.directPdfA * emissionInvPdfW);

        if ((lightFlags & ILight::Flag_IsDelta) == 0)
        {
            const float cosAtLight = outPath.isFiniteLight ? emitResult.cosAtLight : 1.0f;
            outPath.dVC = Mis(cosAtLight * emissionInvPdfW);
        }
        else
        {
            outPath.dVC = 0.0f;
        }

        outPath.dVM = outPath.dVC * mMisVertexConnectionWeightFactorVC;
    }

    return true;
}

bool VertexConnectionAndMerging::AdvancePath(PathState& path, const ShadingData& shadingData, RenderingContext& ctx, PathType pathType) const
{
    Float3 sample;
    if (pathType == PathType::Camera)
    {
        sample = ctx.sampler.GetFloat3();
    }
    else
    {
        sample = ctx.randomGenerator.GetFloat3();
    }

    NFE_ASSERT(sample.x >= 0.0f && sample.x < 1.0f);
    NFE_ASSERT(sample.y >= 0.0f && sample.y < 1.0f);
    NFE_ASSERT(sample.z >= 0.0f && sample.z < 1.0f);

    // sample BSDF
    Vector4 incomingDirWorldSpace;
    float bsdfDirPdf;
    BSDF::EventType sampledEvent = BSDF::NullEvent;
    const RayColor bsdfValue = shadingData.intersection.material->Sample(ctx.wavelength, incomingDirWorldSpace, shadingData, sample, &bsdfDirPdf, &sampledEvent);

    const float cosThetaOut = Abs(Vector4::Dot3(incomingDirWorldSpace, shadingData.intersection.frame[2]));

    if (sampledEvent == BSDF::NullEvent)
    {
        return false;
    }

    NFE_ASSERT(bsdfValue.IsValid());

    path.throughput *= bsdfValue;
    if (path.throughput.AlmostZero())
    {
        return false;
    }

    NFE_ASSERT(bsdfDirPdf >= 0.0f);
    NFE_ASSERT(IsValid(bsdfDirPdf));

    // generate secondary ray
    path.ray = Ray(shadingData.intersection.frame.GetTranslation(), incomingDirWorldSpace);
    path.ray.origin += path.ray.dir * 0.001f;
    NFE_ASSERT(path.ray.IsValid());

    path.lastSampledBsdfEvent = sampledEvent;
    path.length++;

    // TODO Russian roulette

    if (sampledEvent & BSDF::SpecularEvent)
    {
        path.dVC *= Mis(cosThetaOut);
        path.dVM *= Mis(cosThetaOut);
        path.dVCM = 0.0f;
        path.lastSpecular = true;
    }
    else
    {
        // TODO for some reason this gives wrong result
        // evaluate reverse PDF
        const BSDF::EvaluationContext evalContext =
        {
            *shadingData.intersection.material,
            shadingData.materialParams,
            ctx.wavelength,
            shadingData.intersection.WorldToLocal(shadingData.outgoingDirWorldSpace),
            -shadingData.intersection.WorldToLocal(incomingDirWorldSpace),
        };

        const float bsdfRevPdf = shadingData.intersection.material->GetBSDF()->Pdf(evalContext, BSDF::ReversePdf);
        const float invBsdfDirPdf = 1.0f / bsdfDirPdf;

        path.dVC = Mis(cosThetaOut * invBsdfDirPdf) * (path.dVC * Mis(bsdfRevPdf) + path.dVCM + mMisVertexMergingWeightFactorVC);
        path.dVM = Mis(cosThetaOut * invBsdfDirPdf) * (path.dVM * Mis(bsdfRevPdf) + path.dVCM * mMisVertexConnectionWeightFactorVC + 1.0f);
        path.dVCM = Mis(invBsdfDirPdf);
        path.lastSpecular = false;
    }

    NFE_ASSERT(IsValid(path.dVC) && path.dVC >= 0.0f);
    NFE_ASSERT(IsValid(path.dVM) && path.dVM >= 0.0f);
    NFE_ASSERT(IsValid(path.dVCM) && path.dVCM >= 0.0f);

    return true;
}

const RayColor VertexConnectionAndMerging::EvaluateLight(uint32 iteration, const LightSceneObject* lightObject, const IntersectionData* intersection, const PathState& pathState, RenderingContext& ctx) const
{
    const Matrix4 worldToLight = lightObject->GetInverseTransform(ctx.time);
    const Ray lightSpaceRay = worldToLight.TransformRay_Unsafe(pathState.ray);
    const float cosAtLight = intersection ? -intersection->CosTheta(pathState.ray.dir) : 1.0f;
    const Vector4 lightSpaceHitPoint = intersection ? worldToLight.TransformPoint(intersection->frame.GetTranslation()) : Vector4::Zero();

    const ILight& light = lightObject->GetLight();

    const ILight::RadianceParam param =
    {
        ctx,
        lightSpaceRay,
        lightSpaceHitPoint,
        cosAtLight,
        false, // rendererSupportsSolidAngleSampling
    };

    float directPdfA, emissionPdfW;
    RayColor lightContribution = light.GetRadiance(param, &directPdfA, &emissionPdfW);
    NFE_ASSERT(lightContribution.IsValid());

    if (lightContribution.AlmostZero())
    {
        return RayColor::Zero();
    }

    NFE_ASSERT(directPdfA >= 0.0f && IsValid(directPdfA));
    NFE_ASSERT(emissionPdfW >= 0.0f && IsValid(emissionPdfW));

    // no weighting required for directly visible lights
    if (pathState.length > 1)
    {
        const bool useVertexMerging = mUseVertexMerging && iteration > 0;
        if (useVertexMerging && !mUseVertexConnection) // special case for photon mapping
        {
            if (!pathState.lastSpecular)
            {
                return RayColor::Zero();
            }
        }
        else
        {
            // TODO Russian roulette

            // compute MIS weight
            const float wCamera = Mis(directPdfA) * pathState.dVCM + Mis(emissionPdfW) * pathState.dVC;
            const float misWeight = 1.0f / (1.0f + wCamera);
            NFE_ASSERT(misWeight >= 0.0f);

            lightContribution *= misWeight;
        }
    }

    lightContribution *= RayColor::ResolveRGB(ctx.wavelength, mBSDFSamplingWeight);

    return lightContribution;
}

const RayColor VertexConnectionAndMerging::SampleLight(const Scene& scene, const LightSceneObject* lightObject, const ShadingData& shadingData, const PathState& pathState, RenderingContext& ctx) const
{
    const ILight& light = lightObject->GetLight();

    const ILight::IlluminateParam illuminateParam =
    {
        lightObject->GetInverseTransform(ctx.time),
        lightObject->GetTransform(ctx.time),
        shadingData.intersection,
        ctx.wavelength,
        ctx.sampler.GetFloat3(),
        false, // rendererSupportsSolidAngleSampling
    };

    // calculate light contribution
    ILight::IlluminateResult illuminateResult;
    RayColor radiance = light.Illuminate(illuminateParam, illuminateResult);
    if (radiance.AlmostZero())
    {
        return RayColor::Zero();
    }

    NFE_ASSERT(radiance.IsValid());
    NFE_ASSERT(IsValid(illuminateResult.directPdfW) && illuminateResult.directPdfW >= 0.0f);
    NFE_ASSERT(IsValid(illuminateResult.emissionPdfW) && illuminateResult.emissionPdfW >= 0.0f);
    NFE_ASSERT(IsValid(illuminateResult.distance) && illuminateResult.distance >= 0.0f);
    NFE_ASSERT(IsValid(illuminateResult.cosAtLight) && illuminateResult.cosAtLight >= 0.0f);

    // calculate BSDF contribution
    float bsdfPdfW, bsdfRevPdfW;
    const RayColor bsdfFactor = shadingData.intersection.material->Evaluate(ctx.wavelength, shadingData, -illuminateResult.directionToLight, &bsdfPdfW, &bsdfRevPdfW);
    NFE_ASSERT(bsdfFactor.IsValid());

    if (bsdfFactor.AlmostZero())
    {
        return RayColor::Zero();
    }

    NFE_ASSERT(bsdfPdfW > 0.0f && IsValid(bsdfPdfW));

    // cast shadow ray
    {
        HitPoint hitPoint;
        hitPoint.distance = illuminateResult.distance * 0.999f;

        Ray shadowRay(shadingData.intersection.frame.GetTranslation(), illuminateResult.directionToLight);
        shadowRay.origin += shadowRay.dir * 0.0001f;

        if (scene.Traverse_Shadow({ shadowRay, hitPoint, ctx }))
        {
            // shadow ray missed the light - light is occluded
            return RayColor::Zero();
        }
    }

    // TODO
    //const auto& allLocalLights = mScene.GetLights();
    //const float lightPickProbability = 1.0f / (float)allLocalLights.size();
    const float lightPickProbability = 1.0f;

    // TODO
    const bool isDeltaLight = light.GetFlags() & ILight::Flag_IsDelta;
    const float continuationProbability = 1.0f;
    bsdfPdfW *= isDeltaLight ? 0.0f : continuationProbability;
    bsdfRevPdfW *= continuationProbability;

    const float cosToLight = Vector4::Dot3(shadingData.intersection.frame[2], illuminateResult.directionToLight);
    if (cosToLight <= FLT_EPSILON)
    {
        return RayColor::Zero();
    }

    const float wLight = Mis(bsdfPdfW / (lightPickProbability * illuminateResult.directPdfW));
    const float wCamera = Mis(illuminateResult.emissionPdfW * cosToLight / (illuminateResult.directPdfW * illuminateResult.cosAtLight)) * (mMisVertexMergingWeightFactorVC + pathState.dVCM + pathState.dVC * Mis(bsdfRevPdfW));
    const float misWeight = 1.0f / (wLight + 1.0f + wCamera);
    NFE_ASSERT(misWeight >= 0.0f);

    return (radiance * bsdfFactor) * (misWeight / (lightPickProbability * illuminateResult.directPdfW));
}

const RayColor VertexConnectionAndMerging::SampleLights(const Scene& scene, const ShadingData& shadingData, const PathState& pathState, RenderingContext& ctx) const
{
    RayColor accumulatedColor = RayColor::Zero();

    // TODO check only one (or few) lights per sample instead all of them
    // TODO check only nearest lights
    for (const LightSceneObject* lightObject : scene.GetLights())
    {
        accumulatedColor += SampleLight(scene, lightObject, shadingData, pathState, ctx);
    }

    accumulatedColor *= RayColor::ResolveRGB(ctx.wavelength, mLightSamplingWeight);

    return accumulatedColor;
}

const RayColor VertexConnectionAndMerging::EvaluateGlobalLights(const Scene& scene, uint32 iteration, const PathState& pathState, RenderingContext& ctx) const
{
    RayColor result = RayColor::Zero();

    for (const LightSceneObject* globalLightObject : scene.GetGlobalLights())
    {
        result += EvaluateLight(iteration, globalLightObject, nullptr, pathState, ctx);
    }

    return result;
}

const RayColor VertexConnectionAndMerging::ConnectVertices(const Scene& scene, PathState& cameraPathState, const ShadingData& shadingData, const LightVertex& lightVertex, RenderingContext& ctx) const
{
    // compute connection direction (from camera vertex to light vertex)
    Vector4 lightDir = lightVertex.shadingData.intersection.frame.GetTranslation() - shadingData.intersection.frame.GetTranslation();
    const float distanceSqr = lightDir.SqrLength3();
    const float distance = sqrtf(distanceSqr);
    lightDir /= distance;

    const float cosCameraVertex = shadingData.intersection.CosTheta(lightDir);
    const float cosLightVertex = lightVertex.shadingData.intersection.CosTheta(-lightDir);

    if (cosCameraVertex <= 0.0f || cosLightVertex <= 0.0f)
    {
        // line between vertices is occluded (due to backface culling)
        return RayColor::Zero();
    }

    // compute geometry term
    const float geometryTerm = 1.0f / distanceSqr;

    // evaluate BSDF at camera vertex
    float cameraBsdfPdfW, cameraBsdfRevPdfW;
    const RayColor cameraFactor = shadingData.intersection.material->Evaluate(ctx.wavelength, shadingData, -lightDir, &cameraBsdfPdfW, &cameraBsdfRevPdfW);
    NFE_ASSERT(cameraFactor.IsValid());
    if (cameraFactor.AlmostZero())
    {
        return RayColor::Zero();
    }

    // evaluate BSDF at light vertex
    float lightBsdfPdfW, lightBsdfRevPdfW;
    const RayColor lightFactor = lightVertex.shadingData.intersection.material->Evaluate(ctx.wavelength, lightVertex.shadingData, lightDir, &lightBsdfPdfW, &lightBsdfRevPdfW);
    NFE_ASSERT(lightFactor.IsValid());
    if (lightFactor.AlmostZero())
    {
        return RayColor::Zero();
    }

    //// cast shadow ray
    {
        HitPoint hitPoint;
        hitPoint.distance = distance * 0.999f;

        Ray shadowRay(shadingData.intersection.frame.GetTranslation(), lightDir);
        shadowRay.origin += shadowRay.dir * 0.0001f;

        if (scene.Traverse_Shadow({ shadowRay, hitPoint, ctx }))
        {
            // line between vertices is occluded by other geometry
            return RayColor::Zero();
        }
    }

    // TODO
    const float continuationProbability = 1.0f;
    lightBsdfPdfW *= continuationProbability;
    lightBsdfRevPdfW *= continuationProbability;

    // compute MIS weight
    const float cameraBsdfPdfA = PdfWtoA(cameraBsdfPdfW, distance, cosLightVertex);
    const float lightBsdfPdfA = PdfWtoA(lightBsdfPdfW, distance, cosCameraVertex);

    const float wLight = Mis(cameraBsdfPdfA) * (mMisVertexMergingWeightFactorVC + lightVertex.dVCM + lightVertex.dVC * Mis(lightBsdfRevPdfW));
    NFE_ASSERT(IsValid(wLight) && wLight >= 0.0f);

    const float wCamera = Mis(lightBsdfPdfA) * (mMisVertexMergingWeightFactorVC + cameraPathState.dVCM + cameraPathState.dVC * Mis(cameraBsdfRevPdfW));
    NFE_ASSERT(IsValid(wCamera) && wCamera >= 0.0f);

    const float misWeight = 1.0f / (wLight + 1.0f + wCamera);
    NFE_ASSERT(misWeight >= 0.0f);

    const RayColor contribution = (cameraFactor * lightFactor) * (geometryTerm * misWeight);
    NFE_ASSERT(contribution.IsValid());

    return contribution;
}

const RayColor VertexConnectionAndMerging::MergeVertices(PathState& cameraPathState, const ShadingData& shadingData, RenderingContext& ctx) const
{
    //NFE_SCOPED_TIMER(MergeVertices);

    class RangeQuery
    {
    public:
        NFE_FORCE_INLINE RangeQuery(const VertexConnectionAndMerging& renderer, const PathState& cameraPathState, const ShadingData& shadingData, RenderingContext& ctx)
            : mRenderer(renderer)
            , mShadingData(shadingData)
            , mCameraPathState(cameraPathState)
            , mContext(ctx)
            , mContribution(RayColor::Zero())
        {}

        NFE_FORCE_INLINE const RayColor& GetContribution() const { return mContribution; }

        NFE_FORCE_NOINLINE void operator()(uint32 photonIndex)
        {
            const Photon& photon = mRenderer.mPhotons[photonIndex];

            // TODO russian roulette
            //if (photon.pathLength + mCameraPathState.length > mRenderer.mMaxPathLength)
            //{
            //    return;
            //}

            // decompress light incoming direction in world coordinates
            const Vector4 lightDirection{ photon.direction.ToVector() };
            NFE_ASSERT(lightDirection.IsValid());

            const float cosToLight = mShadingData.intersection.CosTheta(lightDirection);
            if (cosToLight < FLT_EPSILON)
            {
                return;
            }

            float cameraBsdfDirPdfW, cameraBsdfRevPdfW;
            const RayColor cameraBsdfFactor = mShadingData.intersection.material->Evaluate(mContext.wavelength, mShadingData, -lightDirection, &cameraBsdfDirPdfW, &cameraBsdfRevPdfW);
            NFE_ASSERT(cameraBsdfFactor.IsValid());
            
            if (cameraBsdfFactor.AlmostZero())
            {
                return;
            }

            // decompress photon throughput
            const RayColor throughput = RayColor::ResolveRGB(mContext.wavelength, photon.throughput.ToVector());
            NFE_ASSERT(throughput.IsValid());

            // TODO russian roulette
            //cameraBsdfDirPdfW *= mCameraBsdf.ContinuationProb();
            //cameraBsdfRevPdfW *= aLightVertex.mBSDF.ContinuationProb();

            // Partial light sub-path MIS weight [tech. rep. (38)]
            const float wLight = photon.dVCM * mRenderer.mMisVertexConnectionWeightFactorVM + photon.dVM * Mis(cameraBsdfDirPdfW);
            const float wCamera = mCameraPathState.dVCM * mRenderer.mMisVertexConnectionWeightFactorVM + mCameraPathState.dVM * Mis(cameraBsdfRevPdfW);
            const float misWeight = 1.0f / (wLight + 1.0f + wCamera);
            const float weight = misWeight / cosToLight;
            NFE_ASSERT(IsValid(weight));
            NFE_ASSERT(weight > 0.0f);

            mContribution.MulAndAccumulate(cameraBsdfFactor * throughput, weight);
        }

    private:
        const VertexConnectionAndMerging& mRenderer;
        const ShadingData& mShadingData;
        const PathState& mCameraPathState;
        RenderingContext& mContext;
        RayColor mContribution;
    };

    const Vector4& cameraVertexPos = shadingData.intersection.frame.GetTranslation();

    RangeQuery query(*this, cameraPathState, shadingData, ctx);
#ifdef NFE_VCM_USE_KD_TREE
    mKdTree.Find(cameraVertexPos, mMergingRadiusVM, mPhotons, query);
#else
    mHashGrid.Process(cameraVertexPos, mPhotons, query);
#endif // NFE_VCM_USE_KD_TREE

    return query.GetContribution();
}

void VertexConnectionAndMerging::ConnectToCamera(const RenderParam& renderParams, const LightVertex& lightVertex, RenderingContext& ctx) const
{
    const Vector4 cameraPos = renderParams.camera.GetTransform().GetTranslation();
    const Vector4 samplePos = lightVertex.shadingData.intersection.frame.GetTranslation();

    Vector4 dirToCamera = cameraPos - samplePos;

    const float cameraDistanceSqr = dirToCamera.SqrLength3();
    const float cameraDistance = sqrtf(cameraDistanceSqr);

    dirToCamera /= cameraDistance;

    // calculate BSDF contribution
    float bsdfPdfW, bsdfRevPdfW;
    const RayColor cameraFactor = lightVertex.shadingData.intersection.material->Evaluate(ctx.wavelength, lightVertex.shadingData, -dirToCamera, &bsdfPdfW, &bsdfRevPdfW);
    NFE_ASSERT(cameraFactor.IsValid());

    if (cameraFactor.AlmostZero())
    {
        return;
    }

    Vector4 filmPos;
    if (!renderParams.camera.WorldToFilm(samplePos, filmPos))
    {
        // vertex is not visible in the viewport
        return;
    }

    HitPoint shadowHitPoint;
    shadowHitPoint.distance = cameraDistance * 0.999f;

    Ray shadowRay(samplePos, dirToCamera);
    shadowRay.origin += shadowRay.dir * 0.0001f;

    if (renderParams.scene.Traverse_Shadow({ shadowRay, shadowHitPoint, ctx }))
    {
        // vertex is occluded
        return;
    }

    const float cosToCamera = Vector4::Dot3(dirToCamera, lightVertex.shadingData.intersection.frame[2]);
    if (cosToCamera <= FLT_EPSILON)
    {
        return;
    }

    const float cameraPdfW = renderParams.camera.PdfW(-dirToCamera);
    const float cameraPdfA = cameraPdfW * cosToCamera / cameraDistanceSqr;

    // compute MIS weight
    const float wLight = Mis(cameraPdfA) * (mMisVertexMergingWeightFactorVC + lightVertex.dVCM + lightVertex.dVC * Mis(bsdfRevPdfW));
    const float misWeight = 1.0f / (wLight + 1.0f);
    NFE_ASSERT(misWeight >= 0.0f);

    RayColor contribution = (cameraFactor * lightVertex.throughput) * (misWeight * cameraPdfA / (cosToCamera));
    contribution *= RayColor::ResolveRGB(ctx.wavelength, mCameraConnectingWeight);

    const Vector4 value = contribution.ConvertToTristimulus(ctx.wavelength);
    renderParams.film.AccumulateColor(filmPos, value, ctx.randomGenerator);
}

} // namespace RT
} // namespace NFE

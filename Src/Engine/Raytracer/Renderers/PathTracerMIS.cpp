#include "PCH.h"
#include "PathTracerMIS.h"
#include "../Rendering/RenderingContext.h"
#include "../Rendering/RenderingParams.h"
#include "../Rendering/PathDebugging.h"
#include "Scene/Scene.h"
#include "Scene/Light/Light.h"
#include "Scene/Object/SceneObject_Light.h"
#include "Scene/Object/SceneObject_Shape.h"
#include "Material/Material.h"
#include "Traversal/TraversalContext.h"
#include "Sampling/GenericSampler.h"
#include "../Common/Reflection/ReflectionUtils.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::PathTracerMIS)
{
    NFE_CLASS_PARENT(NFE::RT::IRenderer);
    NFE_CLASS_MEMBER(lightSamplingWeight);
    NFE_CLASS_MEMBER(BSDFSamplingWeight);
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

NFE_FORCE_INLINE static float Mis(const float samplePdf)
{
    return samplePdf;
}

NFE_FORCE_INLINE static float CombineMis(const float samplePdf, const float otherPdf)
{
    return FastDivide(Mis(samplePdf), Mis(samplePdf) + Mis(otherPdf));
}

NFE_FORCE_INLINE static float PdfAtoW(const float pdfA, const float distance, const float cosThere)
{
    return FastDivide(pdfA * Sqr(distance), Abs(cosThere));
}

PathTracerMIS::PathTracerMIS()
    : lightSamplingWeight(LdrColorRGB::White())
    , BSDFSamplingWeight(LdrColorRGB::White())
{

}

const RayColor PathTracerMIS::SampleLight(const Scene& scene, const HitPoint& hitPoint, const LightSceneObject* lightObject, const ShadingData& shadingData, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const
{
    const ILight& light = lightObject->GetLight();

    const ILight::IlluminateParam illuminateParam =
    {
        lightObject->GetInverseTransform(context.time),
        lightObject->GetTransform(context.time),
        shadingData.intersection,
        context.wavelength,
        context.sampler.GetVec3f(),
    };

    // calculate light contribution
    ILight::IlluminateResult illuminateResult;
    const RayColor radiance = light.Illuminate(illuminateParam, illuminateResult);
    NFE_ASSERT(radiance.IsValid(), "");

    if (radiance.AlmostZero())
    {
        return RayColor::Zero();
    }

    NFE_ASSERT(IsValid(illuminateResult.directPdfW) && illuminateResult.directPdfW >= 0.0f, "");
    NFE_ASSERT(IsValid(illuminateResult.distance) && illuminateResult.distance >= 0.0f, "");
    NFE_ASSERT(IsValid(illuminateResult.cosAtLight) && illuminateResult.cosAtLight >= 0.0f, "");
    NFE_ASSERT(illuminateResult.directionToLight.IsValid(), "");

    // calculate BSDF contribution
    float bsdfPdfW;
    const RayColor factor = shadingData.intersection.material->Evaluate(context.sampler, context.wavelength, shadingData, -illuminateResult.directionToLight, &bsdfPdfW);
    NFE_ASSERT(factor.IsValid(), "");

    if (factor.AlmostZero())
    {
        return RayColor::Zero();
    }

    NFE_ASSERT(bsdfPdfW >= 0.0f && IsValid(bsdfPdfW), "");

    // cast shadow ray
    {
        HitPoint shadowHitPoint;
        shadowHitPoint.objectId = hitPoint.objectId;
        shadowHitPoint.subObjectId = hitPoint.subObjectId;
        shadowHitPoint.distance = illuminateResult.distance * 0.999f; // TODO get rid of that

        Ray shadowRay(shadingData.intersection.frame.GetTranslation(), illuminateResult.directionToLight);
        shadowRay.origin += shadowRay.dir * 0.001f; // TODO get rid of that

        context.counters.numShadowRays++;
        if (scene.Traverse_Shadow({ shadowRay, shadowHitPoint, context }))
        {
            // shadow ray missed the light - light is occluded
            return RayColor::Zero();
        }
        else
        {
            context.counters.numShadowRaysHit++;
        }
    }

    float weight = 1.0f;

    // bypass MIS when this is the last path sample so the energy is not lost
    // TODO this does not include russian roulette
    const bool isLastPathSegment = pathState.depth >= context.params->maxRayDepth;

    const ILight::Flags lightFlags = light.GetFlags();
    if (!(lightFlags & ILight::Flag_IsDelta) && !isLastPathSegment)
    {
        // TODO this should be based on material color
        const float continuationProbability = 1.0f;

        bsdfPdfW *= continuationProbability;
        weight = CombineMis(illuminateResult.directPdfW * lightPickProbability, bsdfPdfW);
    }

    const RayColor result = (radiance * factor) * FastDivide(weight, lightPickProbability * illuminateResult.directPdfW);
    NFE_ASSERT(result.IsValid(), "");

    return result;
}

const RayColor PathTracerMIS::SampleLights(const Scene& scene, const HitPoint& hitPoint, const ShadingData& shadingData, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const
{
    RayColor accumulatedColor = RayColor::Zero();

    const auto& lights = scene.GetLights();
    if (!lights.Empty())
    {
        switch (context.params->lightSamplingStrategy)
        {
            case LightSamplingStrategy::Single:
            {
                const uint32 lightIndex = context.randomGenerator.GetInt() % lights.Size();
                accumulatedColor = SampleLight(scene, hitPoint, lights[lightIndex], shadingData, pathState, context, lightPickProbability);
                break;
            }

            case LightSamplingStrategy::All:
            {
                for (const LightSceneObject* lightObject : lights)
                {
                    accumulatedColor += SampleLight(scene, hitPoint, lightObject, shadingData, pathState, context, lightPickProbability);
                }
                break;
            }
        };

        accumulatedColor *= RayColor::ResolveRGB(context.wavelength, lightSamplingWeight);
    }

    return accumulatedColor;
}

float PathTracerMIS::GetLightPickingProbability(const Scene& scene, RenderingContext& context) const
{
    switch (context.params->lightSamplingStrategy)
    {
    case LightSamplingStrategy::Single:
        return 1.0f / (float)scene.GetLights().Size();

    case LightSamplingStrategy::All:
        return 1.0f;

    default:
        NFE_FATAL("Invalid light sampling strategy");
    };

    return 0.0f;
}

const RayColor PathTracerMIS::EvaluateLight(const LightSceneObject* lightObject, const Math::Ray& ray, float dist, const IntersectionData& intersection, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const
{
    const ILight& light = lightObject->GetLight();

    const Matrix4 worldToLight = lightObject->GetInverseTransform(context.time);
    const Ray lightSpaceRay = worldToLight.TransformRay_Unsafe(ray);
    const Vec4f lightSpaceHitPoint = worldToLight.TransformPoint(intersection.frame.GetTranslation());
    const float cosAtLight = -intersection.CosTheta(ray.dir);

    const ILight::RadianceParam param =
    {
        context,
        lightSpaceRay,
        lightSpaceHitPoint,
        cosAtLight,
    };

    float directPdfA;
    RayColor lightContribution = light.GetRadiance(param, &directPdfA);
    NFE_ASSERT(lightContribution.IsValid(), "");

    if (lightContribution.AlmostZero())
    {
        return RayColor::Zero();
    }

    NFE_ASSERT(directPdfA > 0.0f, "");

    float misWeight = 1.0f;
    if (pathState.depth > 0 && !pathState.lastSpecular)
    {
        if (IsValid(directPdfA))
        {
            const float directPdfW = PdfAtoW(directPdfA, dist, cosAtLight);
            misWeight = CombineMis(pathState.lastPdfW, directPdfW * lightPickProbability);
        }
        else
        {
            misWeight = 0.0f;
        }
    }

    lightContribution *= RayColor::ResolveRGB(context.wavelength, BSDFSamplingWeight);

    return lightContribution * misWeight;
}

const RayColor PathTracerMIS::EvaluateGlobalLights(const Scene& scene, const Ray& ray, const PathState& pathState, RenderingContext& context, const float lightPickProbability) const
{
    RayColor result = RayColor::Zero();

    for (const LightSceneObject* globalLightObject : scene.GetGlobalLights())
    {
        const Matrix4 worldToLight = globalLightObject->GetInverseTransform(context.time);
        const Ray lightSpaceRay = worldToLight.TransformRay_Unsafe(ray);

        const ILight& light = globalLightObject->GetLight();

        const ILight::RadianceParam param =
        {
            context,
            lightSpaceRay,
        };

        float directPdfW;
        RayColor lightContribution = light.GetRadiance(param, &directPdfW);
        NFE_ASSERT(lightContribution.IsValid(), "");

        if (!lightContribution.AlmostZero())
        {
            NFE_ASSERT(directPdfW > 0.0f && IsValid(directPdfW), "");

            float misWeight = 1.0f;
            if (pathState.depth > 0 && !pathState.lastSpecular)
            {
                misWeight = CombineMis(pathState.lastPdfW, directPdfW * lightPickProbability);
            }

            result.MulAndAccumulate(lightContribution, misWeight);
        }
    }

    result *= RayColor::ResolveRGB(context.wavelength, BSDFSamplingWeight);

    return result;
}

const RayColor PathTracerMIS::RenderPixel(const Math::Ray& primaryRay, const RenderParam& param, RenderingContext& context) const
{
    HitPoint hitPoint;
    Ray ray = primaryRay;

    ShadingData shadingData;

    RayColor resultColor = RayColor::Zero();
    RayColor throughput = RayColor::One();
    BSDF::EventType lastSampledBsdfEvent = BSDF::NullEvent;
    PathTerminationReason pathTerminationReason = PathTerminationReason::None;

    PathState pathState;

    const ISceneObject* sceneObject = nullptr;
    const IMedium* currentMedium = param.scene.GetMediumAtPoint(context, primaryRay.origin);

    const float lightPickProbability = GetLightPickingProbability(param.scene, context);

#ifndef NFE_CONFIGURATION_FINAL
    const auto reportHitPoint = [&]()
    {
        PathDebugData::HitPointData data;
        data.rayOrigin = ray.origin;
        data.rayDir = ray.dir;
        data.hitPoint = hitPoint;
        data.objectHit = sceneObject;
        data.shadingData = shadingData;
        data.throughput = throughput;
        data.bsdfEvent = lastSampledBsdfEvent;
        data.medium = currentMedium;
        context.pathDebugData->data.PushBack(data);
    };
#endif // NFE_CONFIGURATION_FINAL

    for (;;)
    {
        hitPoint.Reset();
        //hitPoint.distance = HitPoint::DefaultDistance;
        param.scene.Traverse({ ray, hitPoint, context });

        // ray missed - return background light color
        if (hitPoint.distance == HitPoint::DefaultDistance)
        {
            resultColor.MulAndAccumulate(throughput, EvaluateGlobalLights(param.scene, ray, pathState, context, lightPickProbability));
            pathTerminationReason = PathTerminationReason::HitBackground;
            break;
        }

        if (hitPoint.distance < FLT_MAX)
        {
            param.scene.EvaluateIntersection(ray, hitPoint, context.time, shadingData.intersection);
        }

        sceneObject = param.scene.GetHitObject(hitPoint.objectId);
        NFE_ASSERT(sceneObject, "");

        // we hit a light directly
        if (const LightSceneObject* lightObject = RTTI::Cast<LightSceneObject>(sceneObject))
        {
            const RayColor lightColor = EvaluateLight(lightObject, ray, hitPoint.distance, shadingData.intersection, pathState, context, lightPickProbability);
            NFE_ASSERT(lightColor.IsValid(), "");
            resultColor.MulAndAccumulate(throughput, lightColor);

            pathTerminationReason = PathTerminationReason::HitLight;
            break;
        }

        // fill up structure with shading data
        shadingData.outgoingDirWorldSpace = -ray.dir;
        param.scene.EvaluateShadingData(shadingData, context);

        // handle medium transition
        if (const ShapeSceneObject* shapeObject = RTTI::Cast<ShapeSceneObject>(sceneObject))
        {
            const IMedium* newMedium = shapeObject->GetMedium();

            if (!shapeObject->GetMaterial())
            {
                const bool enter = Vec4f::Dot3(ray.dir, shadingData.intersection.frame[2]) < 0.0f;

                if (enter)
                {
                    currentMedium = newMedium;

                }
                else
                {
                    // TODO pop medium from stack
                    currentMedium = nullptr;
                }

                // TODO get rid of the offset - use ray filters instead
                ray.origin = ray.GetAtDistance(hitPoint.distance + 0.001f);
                pathState.depth++;
                continue;
            }
        }

        // accumulate emission color
        {
            RayColor emissionColor = shadingData.materialParams.emissionColor;
            NFE_ASSERT(emissionColor.IsValid(), "");

            emissionColor *= RayColor::ResolveRGB(context.wavelength, BSDFSamplingWeight);

            resultColor.MulAndAccumulate(throughput, emissionColor);
            NFE_ASSERT(resultColor.IsValid(), "");
        }

        // sample lights directly (a.k.a. next event estimation)
        resultColor.MulAndAccumulate(throughput, SampleLights(param.scene, hitPoint, shadingData, pathState, context, lightPickProbability));

        // check if the ray depth won't be exeeded in the next iteration
        if (pathState.depth >= context.params->maxRayDepth)
        {
            pathTerminationReason = PathTerminationReason::Depth;
            break;
        }

        // Russian roulette algorithm
        if (pathState.depth >= context.params->minRussianRouletteDepth)
        {
            const float minColorValue = 0.125f;
            float threshold = minColorValue + (1.0f - minColorValue) * shadingData.materialParams.baseColor.Max();
#ifdef NFE_ENABLE_SPECTRAL_RENDERING
            if (context.wavelength.isSingle)
            {
                threshold *= 1.0f / static_cast<float>(Wavelength::NumComponents);
            }
#endif
            if (context.sampler.GetFloat() > threshold)
            {
                pathTerminationReason = PathTerminationReason::RussianRoulette;
                break;
            }
            throughput *= 1.0f / threshold;
            NFE_ASSERT(throughput.IsValid(), "");
        }

        // sample BSDF
        float pdf;
        Vec4f incomingDirWorldSpace;
        const RayColor bsdfValue = shadingData.intersection.material->Sample(context.wavelength, incomingDirWorldSpace, shadingData, context.sampler, &pdf, &lastSampledBsdfEvent);

        if (lastSampledBsdfEvent == BSDF::NullEvent)
        {
            pathTerminationReason = PathTerminationReason::NoSampledEvent;
            break;
        }

        NFE_ASSERT(bsdfValue.IsValid(), "");
        throughput *= bsdfValue;

        // ray is not visible anymore
        if (throughput.AlmostZero())
        {
            pathTerminationReason = PathTerminationReason::Throughput;
            break;
        }

        NFE_ASSERT(pdf >= 0.0f, "");
        pathState.lastSpecular = (lastSampledBsdfEvent & BSDF::SpecularEvent) != 0;
        pathState.lastPdfW = pdf;

        // TODO check for NaNs

#ifndef NFE_CONFIGURATION_FINAL
        if (context.pathDebugData)
        {
            reportHitPoint();
        }
#endif // NFE_CONFIGURATION_FINAL

        // generate secondary ray
        ray = Ray(shadingData.intersection.frame.GetTranslation(), incomingDirWorldSpace);
        ray.origin += ray.dir * 0.001f; // TODO get rid of that

        pathState.depth++;
    }

#ifndef NFE_CONFIGURATION_FINAL
    if (context.pathDebugData)
    {
        reportHitPoint();
        context.pathDebugData->terminationReason = pathTerminationReason;
    }
#endif // NFE_CONFIGURATION_FINAL

    context.counters.numRays += pathState.depth + 1;

    return resultColor;
}

} // namespace RT
} // namespace NFE

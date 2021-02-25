#include "PCH.h"
#include "PathTracer.h"
#include "../Rendering/RenderingContext.h"
#include "../Rendering/RenderingParams.h"
#include "../Rendering/PathDebugging.h"
#include "Scene/Scene.h"
#include "Scene/Light/Light.h"
#include "Scene/Object/SceneObject.h"
#include "Scene/Object/SceneObject_Light.h"
#include "Scene/Object/SceneObject_Shape.h"
#include "Medium/Medium.h"
#include "Material/Material.h"
#include "Traversal/TraversalContext.h"
#include "Sampling/GenericSampler.h"
#include "../Common/Reflection/ReflectionUtils.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::PathTracer)
    NFE_CLASS_PARENT(NFE::RT::IRenderer)
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

PathTracer::PathTracer()
{
}

const RayColor PathTracer::EvaluateLight(const LightSceneObject* lightObject, const Math::Ray& ray, const IntersectionData& intersection, RenderingContext& context) const
{
    const float cosAtLight = -intersection.CosTheta(ray.dir);

    const Matrix4 worldToLight = lightObject->GetInverseTransform(context.time);
    const Ray lightSpaceRay = worldToLight.TransformRay_Unsafe(ray);
    const Vec4f lightSpaceHitPoint = worldToLight.TransformPoint(intersection.frame.GetTranslation());

    const ILight& light = lightObject->GetLight();

    const ILight::RadianceParam param =
    {
        context,
        lightSpaceRay,
        lightSpaceHitPoint,
        cosAtLight,
    };

    return light.GetRadiance(param);
}

const RayColor PathTracer::EvaluateGlobalLights(const Scene& scene, const Ray& ray, RenderingContext& context) const
{
    RayColor result = RayColor::Zero();

    for (const LightSceneObject* globalLightObject : scene.GetGlobalLights())
    {
        const Matrix4 worldToLight = globalLightObject->GetInverseTransform(context.time);
        const Ray lightSpaceRay = worldToLight.TransformRay_Unsafe(ray);

        const ILight& globalLight = globalLightObject->GetLight();

        const ILight::RadianceParam param =
        {
            context,
            lightSpaceRay,
        };

        RayColor lightContribution = globalLight.GetRadiance(param);
        NFE_ASSERT(lightContribution.IsValid(), "");

        result += lightContribution;
    }

    return result;
}

const RayColor PathTracer::RenderPixel(const Math::Ray& primaryRay, const RenderParam& param, RenderingContext& context) const
{
    HitPoint hitPoint;
    Ray ray = primaryRay;

    ShadingData shadingData;

    RayColor resultColor = RayColor::Zero();
    RayColor throughput = RayColor::One();
    BSDF::EventType lastSampledBsdfEvent = BSDF::NullEvent;
    PathTerminationReason pathTerminationReason = PathTerminationReason::None;

    uint32 depth = 0;

    const ISceneObject* sceneObject = nullptr;
    const IMedium* currentMedium = param.scene.GetMediumAtPoint(context, primaryRay.origin);

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

        // sample medium first
        if (currentMedium)
        {
            MediumScatteringEvent event;
            const RayColor mediumWeight = currentMedium->Sample(ray, 0.0f, hitPoint.distance, event, context);
            NFE_ASSERT(mediumWeight.IsValid(), "");

            // HACK
            if (event.radiance.IsValid())
            {
                // medium emission
                resultColor += event.radiance * throughput * mediumWeight;
            }

            // attenuate by the medium
            throughput *= mediumWeight;

            if (throughput.AlmostZero())
            {
                pathTerminationReason = PathTerminationReason::AttenuatedInMedium;
                break;
            }

            if (event.distance < FLT_MAX)
            {
#ifndef NFE_CONFIGURATION_FINAL
                if (context.pathDebugData)
                {
                    reportHitPoint();
                }
#endif // NFE_CONFIGURATION_FINAL

                // generate secondary ray
                const Vec4f scatterPosition = ray.GetAtDistance(event.distance);
                ray = Ray(scatterPosition, event.direction);
                depth++;
                continue;
            }
        }

        // ray missed - return background light color
        if (hitPoint.distance == HitPoint::DefaultDistance)
        {
            resultColor.MulAndAccumulate(throughput, EvaluateGlobalLights(param.scene, ray, context));
            pathTerminationReason = PathTerminationReason::HitBackground;
            break;
        }

        sceneObject = param.scene.GetHitObject(hitPoint.objectId);
        NFE_ASSERT(sceneObject, "");

        // fill up structure with shading data
        param.scene.EvaluateIntersection(ray, hitPoint, context.time, shadingData.intersection);
        shadingData.outgoingDirWorldSpace = -ray.dir;

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
                depth++;
                continue;
            }
        }

        // we hit a light directly
        if (const LightSceneObject* lightObject = RTTI::Cast<LightSceneObject>(sceneObject))
        {
            const RayColor lightColor = EvaluateLight(lightObject, ray, shadingData.intersection, context);
            NFE_ASSERT(lightColor.IsValid(), "");
            resultColor.MulAndAccumulate(throughput, lightColor);

            pathTerminationReason = PathTerminationReason::HitLight;
            break;
        }

        param.scene.EvaluateShadingData(shadingData, context);

        // accumulate emission color
        NFE_ASSERT(shadingData.materialParams.emissionColor.IsValid(), "");
        resultColor.MulAndAccumulate(throughput, shadingData.materialParams.emissionColor);
        NFE_ASSERT(resultColor.IsValid(), "");

        // check if the ray depth won't be exeeded in the next iteration
        if (depth >= context.params->maxRayDepth)
        {
            pathTerminationReason = PathTerminationReason::Depth;
            break;
        }

        // Russian roulette algorithm
        if (depth >= context.params->minRussianRouletteDepth)
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
        Vec4f incomingDirWorldSpace;
        const RayColor bsdfValue = shadingData.intersection.material->Sample(context.wavelength, incomingDirWorldSpace, shadingData, context.sampler, nullptr, &lastSampledBsdfEvent);

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

#ifndef NFE_CONFIGURATION_FINAL
        if (context.pathDebugData)
        {
            reportHitPoint();
        }
#endif // NFE_CONFIGURATION_FINAL

        // generate secondary ray
        ray = Ray(shadingData.intersection.frame.GetTranslation(), incomingDirWorldSpace);
        ray.origin += ray.dir * 0.001f; // TODO get rid of that

        depth++;
    }

#ifndef NFE_CONFIGURATION_FINAL
    if (context.pathDebugData)
    {
        reportHitPoint();
        context.pathDebugData->terminationReason = pathTerminationReason;
    }
#endif // NFE_CONFIGURATION_FINAL

    context.counters.numRays += depth + 1;

    return resultColor;
}

} // namespace RT
} // namespace NFE

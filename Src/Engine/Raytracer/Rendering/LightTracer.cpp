#include "PCH.h"
#include "LightTracer.h"
#include "Film.h"
#include "RenderingContext.h"
#include "RenderingParams.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"
#include "Scene/Light/Light.h"
#include "Scene/Object/SceneObject_Light.h"
#include "Material/Material.h"
#include "Traversal/TraversalContext.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::LightTracer)
    NFE_CLASS_PARENT(NFE::RT::IRenderer)
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

LightTracer::LightTracer()
{
}

const RayColor LightTracer::RenderPixel(const Ray&, const RenderParam& param, RenderingContext& ctx) const
{
    uint32 depth = 0;

    const auto& allLocalLights = param.scene.GetLights();
    if (allLocalLights.Empty())
    {
        // no lights on the scene
        return RayColor::Zero();
    }

    const float lightPickingProbability = 1.0f / (float)allLocalLights.Size();
    const uint32 lightIndex = ctx.randomGenerator.GetInt() % allLocalLights.Size();

    const LightSceneObject* lightObject = allLocalLights[lightIndex];
    const ILight& light = lightObject->GetLight();

    const ILight::EmitParam emitParam =
    {
        lightObject->GetTransform(ctx.time),
        ctx.wavelength,
        ctx.randomGenerator.GetVec3f(),
        ctx.randomGenerator.GetVec2f(),
    };

    ILight::EmitResult emitResult;
    RayColor throughput = light.Emit(emitParam, emitResult);

    if (throughput.AlmostZero())
    {
        // generated too weak sample - skip it
        return RayColor::Zero();
    }

    emitResult.emissionPdfW *= lightPickingProbability;
    NFE_ASSERT(emitResult.emissionPdfW > 0.0f);

    emitResult.position += emitResult.direction * 0.0005f;
    Ray ray = Ray(emitResult.position, emitResult.direction);

    // TODO don't divide by pdf in ILight::Emit()
    throughput *= 1.0f / emitResult.emissionPdfW;

    HitPoint hitPoint;
    ShadingData shadingData;

    for (;;)
    {
        hitPoint.Reset();
        param.scene.Traverse({ ray, hitPoint, ctx });

        if (hitPoint.distance == HitPoint::DefaultDistance)
        {
            break; // ray missed
        }

        const ITraceableSceneObject* sceneObject = param.scene.GetHitObject(hitPoint.objectId);
        if (sceneObject->GetDynamicType()->IsA(RTTI::GetType<LightSceneObject>()))
        {
            break; // we hit a light directly
        }

        // fill up structure with shading data
        if (hitPoint.distance < FLT_MAX)
        {
            param.scene.EvaluateIntersection(ray, hitPoint, ctx.time, shadingData.intersection);
            shadingData.outgoingDirWorldSpace = -ray.dir;

            NFE_ASSERT(shadingData.intersection.material != nullptr);
            shadingData.intersection.material->EvaluateShadingData(ctx.wavelength, shadingData);
        }

        // check if the ray depth won't be exeeded in the next iteration
        if (depth >= ctx.params->maxRayDepth)
        {
            break;
        }

//        // Russian roulette algorithm
//        if (depth >= ctx.params->minRussianRouletteDepth)
//        {
//            float threshold = throughput.Max();
//#ifdef NFE_ENABLE_SPECTRAL_RENDERING
//            if (ctx.wavelength.isSingle)
//            {
//                threshold *= 1.0f / static_cast<float>(Wavelength::NumComponents);
//            }
//#endif
//            if (ctx.randomGenerator.GetFloat() > threshold)
//            {
//                break;
//            }
//            throughput *= 1.0f / threshold;
//        }

        // connect to camera
        {
            const Vec4f cameraPos = param.camera.GetTransform().GetTranslation();
            const Vec4f samplePos = shadingData.intersection.frame.GetTranslation();

            Vec4f dirToCamera = cameraPos - samplePos;

            const float cameraDistanceSqr = dirToCamera.SqrLength3();
            const float cameraDistance = sqrtf(cameraDistanceSqr);

            dirToCamera /= cameraDistance;

            // calculate BSDF contribution
            float bsdfPdfW;
            const RayColor cameraFactor = shadingData.intersection.material->Evaluate(ctx.wavelength, shadingData, -dirToCamera, &bsdfPdfW);
            NFE_ASSERT(cameraFactor.IsValid());

            if (!cameraFactor.AlmostZero())
            {
                Vec4f filmPos;

                if (param.camera.WorldToFilm(samplePos, filmPos))
                {
                    HitPoint shadowHitPoint;
                    shadowHitPoint.distance = cameraDistance * 0.999f;

                    const Ray shadowRay(samplePos + shadingData.intersection.frame[2] * 0.0001f, dirToCamera);

                    if (!param.scene.Traverse_Shadow({ shadowRay, shadowHitPoint, ctx }))
                    {
                        const float cameraPdfA = param.camera.PdfW(-dirToCamera) / cameraDistanceSqr;
                        const RayColor contribution = (cameraFactor * throughput) * cameraPdfA;
                        const Vec4f value = contribution.ConvertToTristimulus(ctx.wavelength);
                        param.film.AccumulateColor(filmPos, value, ctx.randomGenerator);
                    }
                }
            }
        }

        // sample BSDF
        Vec4f incomingDirWorldSpace;
        const RayColor bsdfValue = shadingData.intersection.material->Sample(ctx.wavelength, incomingDirWorldSpace, shadingData, ctx.randomGenerator.GetVec3f());

        NFE_ASSERT(bsdfValue.IsValid());
        throughput *= bsdfValue;

        // ray is not visible anymore
        if (throughput.AlmostZero())
        {
            break;
        }

        // generate secondary ray
        ray = Ray(shadingData.intersection.frame.GetTranslation(), incomingDirWorldSpace);
        ray.origin += ray.dir * 0.001f;

        depth++;
    }

    ctx.counters.numRays += (uint64)depth + 1;

    return RayColor::Zero();
}

} // namespace RT
} // namespace NFE

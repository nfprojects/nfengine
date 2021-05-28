#include "PCH.h"
#include "DebugRenderer.h"
#include "Scene/Scene.h"
#include "Scene/Object/SceneObject_Light.h"
#include "Material/Material.h"
#include "Traversal/TraversalContext.h"
#include "Rendering/Film.h"
#include "Rendering/RenderingContext.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"
#include "../Common/Math/ColorHelpers.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::DebugRenderer)
{
    NFE_CLASS_PARENT(NFE::RT::IRenderer);
    NFE_CLASS_MEMBER(renderingMode);
}
NFE_END_DEFINE_CLASS()

NFE_BEGIN_DEFINE_ENUM(NFE::RT::DebugRenderingMode)
    NFE_ENUM_OPTION(CameraLight)
    NFE_ENUM_OPTION(TriangleID)
    NFE_ENUM_OPTION(Depth)
    NFE_ENUM_OPTION(Position)
    NFE_ENUM_OPTION(Normals)
    NFE_ENUM_OPTION(Tangents)
    NFE_ENUM_OPTION(Bitangents)
    NFE_ENUM_OPTION(TexCoords)
    NFE_ENUM_OPTION(BaseColor)
    NFE_ENUM_OPTION(Emission)
    NFE_ENUM_OPTION(Roughness)
    NFE_ENUM_OPTION(Metalness)
    NFE_ENUM_OPTION(IoR)
#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
    NFE_ENUM_OPTION(RayBoxIntersection)
    NFE_ENUM_OPTION(RayBoxIntersectionPassed)
    NFE_ENUM_OPTION(RayTriIntersection)
    NFE_ENUM_OPTION(RayTriIntersectionPassed)
#endif // NFE_ENABLE_INTERSECTION_COUNTERS
NFE_END_DEFINE_ENUM()

namespace NFE {
namespace RT {

using namespace Math;

DebugRenderer::DebugRenderer()
    : renderingMode(DebugRenderingMode::TriangleID)
{
}

const char* DebugRenderer::GetName() const
{
    return "Debug";
}

const RayColor DebugRenderer::RenderPixel(const Math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const
{
    HitPoint hitPoint;
    param.scene.Traverse({ ray, hitPoint, ctx });

    // traversal tatistics
#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
    if (renderingMode == DebugRenderingMode::RayBoxIntersection)
    {
        const float num = static_cast<float>(ctx.localCounters.numRayBoxTests);
        const Vec4f resultColor = Vec4f(num * 0.01f, num * 0.004f, num * 0.001f, 0.0f);
        return RayColor::ResolveRGB(ctx.wavelength, Spectrum(resultColor));
    }
    else if (renderingMode == DebugRenderingMode::RayBoxIntersectionPassed)
    {
        const float num = static_cast<float>(ctx.localCounters.numPassedRayBoxTests);
        const Vec4f resultColor = Vec4f(num * 0.01f, num * 0.005f, num * 0.001f, 0.0f);
        return RayColor::ResolveRGB(ctx.wavelength, Spectrum(resultColor));
    }
    else if (renderingMode == DebugRenderingMode::RayTriIntersection)
    {
        const float num = static_cast<float>(ctx.localCounters.numRayTriangleTests);
        const Vec4f resultColor = Vec4f(num * 0.01f, num * 0.004f, num * 0.001f, 0.0f);
        return RayColor::ResolveRGB(ctx.wavelength, Spectrum(resultColor));
    }
    else if (renderingMode == DebugRenderingMode::RayTriIntersectionPassed)
    {
        const float num = static_cast<float>(ctx.localCounters.numPassedRayTriangleTests);
        const Vec4f resultColor = Vec4f(num * 0.01f, num * 0.004f, num * 0.001f, 0.0f);
        return RayColor::ResolveRGB(ctx.wavelength, Spectrum(resultColor));
    }
#endif // NFE_ENABLE_INTERSECTION_COUNTERS

    if (hitPoint.distance == HitPoint::DefaultDistance)
    {
        // ray hit background
        return RayColor::Zero();
    }

    const ITraceableSceneObject* sceneObject = param.scene.GetHitObject(hitPoint.objectId);
    if (sceneObject->GetDynamicType()->IsA(RTTI::GetType<LightSceneObject>()))
    {
        // ray hit a light
        const Vec4f lightColor{ 1.0, 1.0f, 0.0f };
        return RayColor::ResolveRGB(ctx.wavelength, lightColor);
    }

    ShadingData shadingData;
    if (renderingMode != DebugRenderingMode::TriangleID && renderingMode != DebugRenderingMode::Depth)
    {
        if (hitPoint.distance < FLT_MAX)
        {
            param.scene.EvaluateIntersection(ray, hitPoint, ctx.time, shadingData.intersection);
        }
        param.scene.EvaluateShadingData(shadingData, ctx);
    }

    RayColor resultColor;

    switch (renderingMode)
    {
        case DebugRenderingMode::CameraLight:
        {
            const float NdotL = Vec4f::Dot3(ray.dir, shadingData.intersection.frame[2]);
            resultColor = shadingData.materialParams.baseColor * Abs(NdotL);
            break;
        }

        // Geometry
        case DebugRenderingMode::Depth:
        {
            const float invDepth = 1.0f - 1.0f / (1.0f + hitPoint.distance / 10.0f);
            resultColor = RayColor::ResolveRGB(ctx.wavelength, Vec4f(invDepth));
            break;
        }
        case DebugRenderingMode::TriangleID:
        {
            const uint64 hash = Hash((uint64)hitPoint.objectId | ((uint64)hitPoint.subObjectId << 32));
            const float hue = (float)(uint32)hash / (float)UINT32_MAX;
            const float saturation = 0.5f + 0.5f * (float)(uint32)(hash >> 32) / (float)UINT32_MAX;
            const Vec4f rgbColor = HSVtoRGB(hue, saturation, 1.0f);
            resultColor = RayColor::ResolveRGB(ctx.wavelength, rgbColor);
            break;
        }
        case DebugRenderingMode::Tangents:
        {
            const Vec4f rgbColor = Vec4f::Saturate(BipolarToUnipolar(shadingData.intersection.frame[0]));
            resultColor = RayColor::ResolveRGB(ctx.wavelength, rgbColor);
            break;
        }
        case DebugRenderingMode::Bitangents:
        {
            const Vec4f rgbColor = Vec4f::Saturate(BipolarToUnipolar(shadingData.intersection.frame[1]));
            resultColor = RayColor::ResolveRGB(ctx.wavelength, rgbColor);
            break;
        }
        case DebugRenderingMode::Normals:
        {
            const Vec4f rgbColor = Vec4f::Saturate(BipolarToUnipolar(shadingData.intersection.frame[2]));
            resultColor = RayColor::ResolveRGB(ctx.wavelength, rgbColor);
            break;
        }
        case DebugRenderingMode::Position:
        {
            const Vec4f rgbColor = Vec4f::Max(Vec4f::Zero(), shadingData.intersection.frame[3]);
            resultColor = RayColor::ResolveRGB(ctx.wavelength, rgbColor);
            break;
        }
        case DebugRenderingMode::TexCoords:
        {
            const Vec4f rgbColor = Vec4f::Mod1(shadingData.intersection.texCoord & Vec4f::MakeMask<1,1,0,0>());
            resultColor = RayColor::ResolveRGB(ctx.wavelength, rgbColor);
            break;
        }

        // Material
        case DebugRenderingMode::BaseColor:
        {
            resultColor = shadingData.materialParams.baseColor;
            break;
        }
        case DebugRenderingMode::Emission:
        {
            resultColor = shadingData.materialParams.emissionColor;
            break;
        }
        case DebugRenderingMode::Roughness:
        {
            const Vec4f rgbColor = Vec4f(shadingData.materialParams.roughness);
            resultColor = RayColor::ResolveRGB(ctx.wavelength, rgbColor);
            break;
        }
        case DebugRenderingMode::Metalness:
        {
            const Vec4f rgbColor = Vec4f(shadingData.materialParams.metalness);
            resultColor = RayColor::ResolveRGB(ctx.wavelength, rgbColor);
            break;
        }
        case DebugRenderingMode::IoR:
        {
            const Vec4f rgbColor = Vec4f(shadingData.materialParams.IoR);
            resultColor = RayColor::ResolveRGB(ctx.wavelength, rgbColor);
            break;
        }

        default:
        {
            NFE_FATAL("Invalid debug rendering mode");
            resultColor = RayColor::Zero();
        }
    }

    return resultColor;
}

void DebugRenderer::Raytrace_Packet(RayPacket& packet, const RenderParam& param, RenderingContext& context) const
{
    param.scene.Traverse({ packet, context });

    ShadingData shadingData;

    const uint32 numGroups = packet.GetNumGroups();
    for (uint32 i = 0; i < numGroups; ++i)
    {
        Vec4f weights[RayPacket::GroupSize];
        packet.rayWeights[i].Unpack(weights);

        Vec4f rayOrigins[RayPacket::GroupSize];
        Vec4f rayDirs[RayPacket::GroupSize];
        packet.groups[i].rays[0].origin.Unpack(rayOrigins);
        packet.groups[i].rays[0].dir.Unpack(rayDirs);

        for (uint32 j = 0; j < RayPacket::GroupSize; ++j)
        {
            const HitPoint& hitPoint = context.hitPoints[RayPacket::GroupSize * i + j];

            Vec4f color = Vec4f::Zero();

            if (hitPoint.distance != FLT_MAX)
            {
                if (renderingMode != DebugRenderingMode::TriangleID && renderingMode != DebugRenderingMode::Depth)
                {
                    param.scene.EvaluateIntersection(Ray(rayOrigins[j], rayDirs[j]), hitPoint, context.time, shadingData.intersection);
                }

                switch (renderingMode)
                {
                    case DebugRenderingMode::CameraLight:
                    {
                        const float NdotL = Vec4f::Dot3(rayDirs[j], shadingData.intersection.frame[2]);
                        color = Vec4f(Abs(NdotL)); // TODO use texture
                        break;
                    }

                    case DebugRenderingMode::Depth:
                    {
                        const float logDepth = std::max<float>(0.0f, (log2f(hitPoint.distance) + 5.0f) / 10.0f);
                        color = Vec4f(logDepth);
                        break;
                    }
                    case DebugRenderingMode::Tangents:
                    {
                        color = BipolarToUnipolar(shadingData.intersection.frame[0]);
                        break;
                    }
                    case DebugRenderingMode::Bitangents:
                    {
                        color = BipolarToUnipolar(shadingData.intersection.frame[1]);
                        break;
                    }
                    case DebugRenderingMode::Normals:
                    {
                        color = BipolarToUnipolar(shadingData.intersection.frame[2]);
                        break;
                    }
                    case DebugRenderingMode::Position:
                    {
                        color = BipolarToUnipolar(shadingData.intersection.frame.GetTranslation());
                        break;
                    }
                    case DebugRenderingMode::TexCoords:
                    {
                        color = BipolarToUnipolar(shadingData.intersection.texCoord);
                        break;
                    }
                    case DebugRenderingMode::TriangleID:
                    {
                        const uint64 hash = Hash((uint64)hitPoint.objectId | ((uint64)hitPoint.subObjectId << 32));
                        const float hue = (float)(uint32)hash / (float)UINT32_MAX;
                        const float saturation = 0.5f + 0.5f * (float)(uint32)(hash >> 32) / (float)UINT32_MAX;
                        color = weights[j] * HSVtoRGB(hue, saturation, 1.0f);
                        break;
                    }
                }
            }

            // clamp color
            color = Vec4f::Max(Vec4f::Zero(), color);

            const ImageLocationInfo& imageLocation = packet.imageLocations[RayPacket::GroupSize * i + j];
            param.film.AccumulateColor(imageLocation.x, imageLocation.y, color);
        }
    }
}

} // namespace RT
} // namespace NFE

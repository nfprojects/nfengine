#include "PCH.h"
#include "DebugRenderer.h"
#include "Scene/Scene.h"
#include "Material/Material.h"
#include "Color/ColorHelpers.h"
#include "Color/Spectrum.h"
#include "Traversal/TraversalContext.h"
#include "Rendering/Film.h"
#include "Rendering/RenderingContext.h"
#include "../nfCommon/Reflection/ReflectionClassDefine.hpp"

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
        const Vector4 resultColor = Vector4(num * 0.01f, num * 0.004f, num * 0.001f, 0.0f);
        return RayColor::Resolve(ctx.wavelength, Spectrum(resultColor));
    }
    else if (renderingMode == DebugRenderingMode::RayBoxIntersectionPassed)
    {
        const float num = static_cast<float>(ctx.localCounters.numPassedRayBoxTests);
        const Vector4 resultColor = Vector4(num * 0.01f, num * 0.005f, num * 0.001f, 0.0f);
        return RayColor::Resolve(ctx.wavelength, Spectrum(resultColor));
    }
    else if (renderingMode == DebugRenderingMode::RayTriIntersection)
    {
        const float num = static_cast<float>(ctx.localCounters.numRayTriangleTests);
        const Vector4 resultColor = Vector4(num * 0.01f, num * 0.004f, num * 0.001f, 0.0f);
        return RayColor::Resolve(ctx.wavelength, Spectrum(resultColor));
    }
    else if (renderingMode == DebugRenderingMode::RayTriIntersectionPassed)
    {
        const float num = static_cast<float>(ctx.localCounters.numPassedRayTriangleTests);
        const Vector4 resultColor = Vector4(num * 0.01f, num * 0.004f, num * 0.001f, 0.0f);
        return RayColor::Resolve(ctx.wavelength, Spectrum(resultColor));
    }
#endif // NFE_ENABLE_INTERSECTION_COUNTERS

    if (hitPoint.distance == HitPoint::DefaultDistance)
    {
        // ray hit background
        return RayColor::Zero();
    }

    if (hitPoint.subObjectId == NFE_LIGHT_OBJECT)
    {
        // ray hit a light
        const Vector4 lightColor{ 1.0, 1.0f, 0.0f };
        return RayColor::Resolve(ctx.wavelength, Spectrum(lightColor));
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
            const float NdotL = Vector4::Dot3(ray.dir, shadingData.intersection.frame[2]);
            resultColor = shadingData.materialParams.baseColor * Abs(NdotL);
            break;
        }

        // Geometry
        case DebugRenderingMode::Depth:
        {
            const float invDepth = 1.0f - 1.0f / (1.0f + hitPoint.distance / 10.0f);
            resultColor = RayColor::Resolve(ctx.wavelength, Spectrum(Vector4(invDepth)));
            break;
        }
        case DebugRenderingMode::TriangleID:
        {
            const uint64 hash = Hash((uint64)hitPoint.objectId | ((uint64)hitPoint.subObjectId << 32));
            const float hue = (float)(uint32)hash / (float)UINT32_MAX;
            const float saturation = 0.5f + 0.5f * (float)(uint32)(hash >> 32) / (float)UINT32_MAX;
            const Vector4 rgbColor = HSVtoRGB(hue, saturation, 1.0f);
            resultColor = RayColor::Resolve(ctx.wavelength, Spectrum(rgbColor));
            break;
        }
        case DebugRenderingMode::Tangents:
        {
            const Vector4 rgbColor = Vector4::Saturate(BipolarToUnipolar(shadingData.intersection.frame[0]));
            resultColor = RayColor::Resolve(ctx.wavelength, Spectrum(rgbColor));
            break;
        }
        case DebugRenderingMode::Bitangents:
        {
            const Vector4 rgbColor = Vector4::Saturate(BipolarToUnipolar(shadingData.intersection.frame[1]));
            resultColor = RayColor::Resolve(ctx.wavelength, Spectrum(rgbColor));
            break;
        }
        case DebugRenderingMode::Normals:
        {
            const Vector4 rgbColor = Vector4::Saturate(BipolarToUnipolar(shadingData.intersection.frame[2]));
            resultColor = RayColor::Resolve(ctx.wavelength, Spectrum(rgbColor));
            break;
        }
        case DebugRenderingMode::Position:
        {
            const Vector4 rgbColor = Vector4::Max(Vector4::Zero(), shadingData.intersection.frame[3]);
            resultColor = RayColor::Resolve(ctx.wavelength, Spectrum(rgbColor));
            break;
        }
        case DebugRenderingMode::TexCoords:
        {
            const Vector4 rgbColor = Vector4::Mod1(shadingData.intersection.texCoord & Vector4::MakeMask<1,1,0,0>());
            resultColor = RayColor::Resolve(ctx.wavelength, Spectrum(rgbColor));
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
            const Vector4 rgbColor = Vector4(shadingData.materialParams.roughness);
            resultColor = RayColor::Resolve(ctx.wavelength, Spectrum(rgbColor));
            break;
        }
        case DebugRenderingMode::Metalness:
        {
            const Vector4 rgbColor = Vector4(shadingData.materialParams.metalness);
            resultColor = RayColor::Resolve(ctx.wavelength, Spectrum(rgbColor));
            break;
        }
        case DebugRenderingMode::IoR:
        {
            const Vector4 rgbColor = Vector4(shadingData.materialParams.IoR);
            resultColor = RayColor::Resolve(ctx.wavelength, Spectrum(rgbColor));
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
        Vector4 weights[RayPacket::RaysPerGroup];
        packet.rayWeights[i].Unpack(weights);

        Vector4 rayOrigins[RayPacket::RaysPerGroup];
        Vector4 rayDirs[RayPacket::RaysPerGroup];
        packet.groups[i].rays[0].origin.Unpack(rayOrigins);
        packet.groups[i].rays[0].dir.Unpack(rayDirs);

        for (uint32 j = 0; j < RayPacket::RaysPerGroup; ++j)
        {
            const HitPoint& hitPoint = context.hitPoints[RayPacket::RaysPerGroup * i + j];

            Vector4 color = Vector4::Zero();

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
                        const float NdotL = Vector4::Dot3(rayDirs[j], shadingData.intersection.frame[2]);
                        color = shadingData.intersection.material->baseColor.Evaluate(shadingData.intersection.texCoord) * Abs(NdotL);
                        break;
                    }

                    case DebugRenderingMode::Depth:
                    {
                        const float logDepth = std::max<float>(0.0f, (log2f(hitPoint.distance) + 5.0f) / 10.0f);
                        color = Vector4(logDepth);
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
            color = Vector4::Max(Vector4::Zero(), color);

            const ImageLocationInfo& imageLocation = packet.imageLocations[RayPacket::RaysPerGroup * i + j];
            param.film.AccumulateColor(imageLocation.x, imageLocation.y, color);
        }
    }
}

} // namespace RT
} // namespace NFE

#include "PCH.h"
#include "AreaLight.h"
#include "../../Rendering/RenderingContext.h"
#include "../../Rendering/ShadingData.h"
#include "../../Textures/Texture.h"
#include "../../Shapes/Shape.h"
#include "../../Shapes/RectShape.h"
#include "../../../nfCommon/Math/Geometry.hpp"
#include "../../../nfCommon/Math/SamplingHelpers.hpp"
#include "../../../nfCommon/Reflection/ReflectionClassDefine.hpp"
#include "../../../nfCommon/Reflection/Types/ReflectionSharedPtrType.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::AreaLight)
{
    NFE_CLASS_PARENT(NFE::RT::ILight);
    NFE_CLASS_MEMBER(mShape).NonNull();
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;
using namespace Common;

AreaLight::AreaLight()
{
    mShape = MakeSharedPtr<RectShape>();
}

AreaLight::~AreaLight() = default;

AreaLight::AreaLight(ShapePtr shape, const Math::HdrColorRGB& color)
    : ILight(color)
    , mShape(std::move(shape))
{
    mShape->MakeSamplable();
}

const Box AreaLight::GetBoundingBox() const
{
    return mShape->GetBoundingBox();
}

void AreaLight::Traverse(const SingleTraversalContext& context, const uint32 objectID) const
{
    mShape->Traverse(context, objectID);
}

bool AreaLight::Traverse_Shadow(const SingleTraversalContext& context) const
{
    return mShape->Traverse_Shadow(context);
}

const RayColor AreaLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    // TODO
    // sample texture map
    //if (mTexture)
    //{
    //    float pdf;
    //    const Vector4 textureColor = mTexture->Sample(param.sample, uv, &pdf);
    //    color.rgbValues *= textureColor / pdf;
    //}
    //else
    //{
    //    uv = Vector4(param.sample);
    //}

    if (param.rendererSupportsSolidAngleSampling)
    {
        const Vector4 ref = param.worldToLight.TransformPoint(param.intersection.frame.GetTranslation());

        ShapeSampleResult sampleResult;
        if (!mShape->Sample(ref, param.sample, sampleResult))
        {
            return RayColor::Zero();
        }

        outResult.directionToLight = param.lightToWorld.TransformVector(sampleResult.direction);
        outResult.distance = sampleResult.distance;
        outResult.cosAtLight = sampleResult.cosAtSurface;
        outResult.directPdfW = sampleResult.pdf;

        NFE_ASSERT(IsValid(outResult.directPdfW));
        NFE_ASSERT(outResult.directPdfW >= 0.0f);
    }
    else
    {
        // generate random point on the light surface
        Vector4 normalLocalSpace;
        const Vector4 samplePositionLocalSpace = mShape->Sample(param.sample, &normalLocalSpace);
        const Vector4 lightPointWorldSpace = param.lightToWorld.TransformPoint(samplePositionLocalSpace);
        const Vector4 normalWorldSpace = param.lightToWorld.TransformPoint(normalLocalSpace);

        outResult.directionToLight = lightPointWorldSpace - param.intersection.frame.GetTranslation();
        const float sqrDistance = outResult.directionToLight.SqrLength3();

        outResult.distance = sqrtf(sqrDistance);
        outResult.directionToLight /= outResult.distance;

        const float cosNormalDir = Vector4::Dot3(-normalWorldSpace, outResult.directionToLight);
        if (cosNormalDir < NFE_MATH_EPSILON)
        {
            return RayColor::Zero();
        }

        const float invArea = 1.0f / mShape->GetSurfaceArea();
        outResult.cosAtLight = cosNormalDir;
        outResult.directPdfW = invArea * sqrDistance / cosNormalDir;
        outResult.emissionPdfW = cosNormalDir * invArea * NFE_MATH_INV_PI;
    }

    return GetColor()->Resolve(param.wavelength);
}

const RayColor AreaLight::GetRadiance(const RadianceParam& param, float* outDirectPdfA, float* outEmissionPdfW) const
{
    if (param.cosAtLight < NFE_MATH_EPSILON)
    {
        return RayColor::Zero();
    }

    const float invArea = 1.0f / mShape->GetSurfaceArea();

    if (outDirectPdfA)
    {
        if (param.rendererSupportsSolidAngleSampling)
        {
            *outDirectPdfA = mShape->Pdf(param.ray.origin, param.hitPoint);
        }
        else
        {
            *outDirectPdfA = invArea;
        }
    }

    if (outEmissionPdfW)
    {
        *outEmissionPdfW = param.cosAtLight * invArea * NFE_MATH_INV_PI;
    }

    // TODO
    //if (mTexture)
    //{
    //    const Vector4 lightSpaceHitPoint = hitPoint - p0;
    //    const float u = Vector4::Dot3(lightSpaceHitPoint, edge0 * edgeLengthInv0) * edgeLengthInv0;
    //    const float v = Vector4::Dot3(lightSpaceHitPoint, edge1 * edgeLengthInv1) * edgeLengthInv1;
    //    const Vector4 textureCoords(u, v, 0.0f, 0.0f);

    //    color.rgbValues *= mTexture->Evaluate(textureCoords);
    //}

    return GetColor()->Resolve(param.context.wavelength);
}

const RayColor AreaLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    // TODO sample texture, like in Illuminate()

    // generate random point on the light surface
    Vector4 normalLocalSpace;
    const Vector4 samplePositionLocalSpace = mShape->Sample(param.positionSample, &normalLocalSpace);
    outResult.position = param.lightToWorld.TransformPoint(samplePositionLocalSpace);

    Vector4 tangentLocalSpace, bitangentLocalSpace;
    BuildOrthonormalBasis(normalLocalSpace, tangentLocalSpace, bitangentLocalSpace);

    // generate random direction
    const Vector4 randomDir = SamplingHelpers::GetHemishpereCos(param.directionSample);
    const Vector4 dirLocalSpace = randomDir.x * tangentLocalSpace + randomDir.y * bitangentLocalSpace + randomDir.z * normalLocalSpace;
    outResult.direction = param.lightToWorld.TransformVector(dirLocalSpace);

    const float cosAtLight = randomDir.z;
    const float invArea = 1.0f / mShape->GetSurfaceArea();
    outResult.cosAtLight = cosAtLight;
    outResult.directPdfA = invArea;
    outResult.emissionPdfW = invArea * cosAtLight * NFE_MATH_INV_PI;

    // TODO
    // sample texture map
    //if (mTexture)
    //{
    //    color.rgbValues *= mTexture->Evaluate(Vector4(uv));
    //}

    return GetColor()->Resolve(param.wavelength) * cosAtLight;
}

ILight::Flags AreaLight::GetFlags() const
{
    return Flag_IsFinite;
}

} // namespace RT
} // namespace NFE

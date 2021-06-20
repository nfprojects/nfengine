#include "PCH.h"
#include "BackgroundLight.h"
#include "../../Rendering/RenderingContext.h"
#include "../../Rendering/ShadingData.h"
#include "../../Textures/Texture.h"
#include "../../../Common/Math/Geometry.hpp"
#include "../../../Common/Math/SamplingHelpers.hpp"
#include "../../../Common/Math/Transcendental.hpp"
#include "../../../Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::BackgroundLight)
{
    NFE_CLASS_PARENT(NFE::RT::ILight);
    // NFE_CLASS_MEMBER(mTexture); // TODO
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Math;

// TODO this should be calculated
static const float SceneRadius = 20.0f; // TODO

BackgroundLight::BackgroundLight() = default;

BackgroundLight::BackgroundLight(const Math::HdrColorRGB& color)
    : ILight(color)
{}

void BackgroundLight::SetTexture(const TexturePtr& texture)
{
    mTexture = texture;
    
    if (mTexture)
    {
        mTexture->MakeSamplable(SampleDistortion::Spherical);
    }
}

const Box BackgroundLight::GetBoundingBox() const
{
    return Box::Full();
}

const RayColor BackgroundLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    // TODO include light rotation

    RayColor color = GetColor()->Resolve(param.wavelength);

    // importance sample texture
    if (mTexture && mTexture->IsSamplable(SampleDistortion::Spherical))
    {
        float pdf;
        Vec4f texCoords;
        const Vec4f textureColor = mTexture->Sample(param.sample, texCoords, SampleDistortion::Spherical, &pdf);
        NFE_ASSERT(pdf > 0.0f && IsValid(pdf), "Invalid PDF");

        outResult.directionToLight = SphericalToCartesian(texCoords.x, texCoords.y);

        const float sinTheta = sinf(texCoords.y * NFE_MATH_PI);
        if (sinTheta < FLT_EPSILON)
        {
            return RayColor::Zero();
        }

        outResult.directPdfW = pdf / NFE_MATH_2PI / NFE_MATH_PI / sinTheta;
        outResult.emissionPdfW = outResult.directPdfW;
        outResult.distance = BackgroundLightDistance;
        outResult.cosAtLight = 1.0f;

        NFE_ASSERT(IsValid(outResult.directPdfW) && outResult.directPdfW >= 0.0f, "");
        NFE_ASSERT(IsValid(outResult.emissionPdfW) && outResult.emissionPdfW >= 0.0f, "");

        color *= RayColor::ResolveRGB(param.wavelength, textureColor);
    }
    else
    {
        const Vec4f randomDirLocalSpace = SamplingHelpers::GetHemishpere(param.sample);
        outResult.directionToLight = param.intersection.LocalToWorld(randomDirLocalSpace);
        outResult.directPdfW = UniformHemispherePdf();
        outResult.emissionPdfW = UniformSpherePdf();
        outResult.distance = BackgroundLightDistance;
        outResult.cosAtLight = 1.0f;

        if (mTexture)
        {
            const Vec4f coords = CartesianToSpherical(outResult.directionToLight);
            NFE_ASSERT(coords.IsValid(), "");

            const Vec4f textureColor = Vec4f::Max(Vec4f::Zero(), mTexture->Evaluate(coords));
            NFE_ASSERT(textureColor.IsValid(), "");

            color *= RayColor::ResolveRGB(param.wavelength, textureColor);
        }
    }

    outResult.emissionPdfW *= UniformCirclePdf(SceneRadius);

    return color;
}

const RayColor BackgroundLight::GetRadiance(const RadianceParam& param, float* outDirectPdfA, float* outEmissionPdfW) const
{
    // TODO include light rotation

    RayColor color = GetColor()->Resolve(param.context.wavelength);

    float directPdfA = UniformHemispherePdf();
    float emissionPdfW = UniformHemispherePdf();

    // sample environment map
    if (mTexture)
    {
        const Vec4f coords = CartesianToSpherical(param.ray.dir);
        NFE_ASSERT(coords.IsValid(), "");

        const Vec4f textureColor = Vec4f::Max(Vec4f::Zero(), mTexture->Evaluate(coords));
        NFE_ASSERT(textureColor.IsValid(), "");

        color *= RayColor::ResolveRGB(param.context.wavelength, textureColor);

        if ((outDirectPdfA || outEmissionPdfW) && mTexture->IsSamplable(SampleDistortion::Spherical))
        {
            const float pdf = mTexture->Pdf(SampleDistortion::Spherical, coords);
            NFE_ASSERT(pdf >= 0.0f && IsValid(pdf), "");

            const float sinTheta = sinf(coords.y * NFE_MATH_PI);
            if (sinTheta < FLT_EPSILON)
            {
                return RayColor::Zero();
            }

            directPdfA = pdf / NFE_MATH_2PI / NFE_MATH_PI / sinTheta;
            emissionPdfW = pdf / NFE_MATH_2PI / NFE_MATH_PI / sinTheta;
        }
    }

    emissionPdfW *= UniformCirclePdf(SceneRadius);

    if (outDirectPdfA) *outDirectPdfA = directPdfA;
    if (outEmissionPdfW) *outEmissionPdfW = emissionPdfW;

    return color;
}

const RayColor BackgroundLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    // TODO include light rotation

    RayColor color = GetColor()->Resolve(param.wavelength);

    // importance sample texture
    if (mTexture && mTexture->IsSamplable(SampleDistortion::Spherical))
    {
        float pdf;
        Vec4f texCoords;
        const Vec4f textureColor = mTexture->Sample(param.directionSample, texCoords, SampleDistortion::Spherical, &pdf);
        NFE_ASSERT(pdf > 0.0f && IsValid(pdf), "Invalid PDF");

        // negate as incoming direction is opposite
        outResult.direction = -SphericalToCartesian(texCoords.x, texCoords.y);

        const float sinTheta = sinf(texCoords.y * NFE_MATH_PI);
        if (sinTheta < FLT_EPSILON)
        {
            return RayColor::Zero();
        }

        outResult.directPdfA = pdf / NFE_MATH_2PI / NFE_MATH_PI / sinTheta;
        outResult.emissionPdfW = outResult.directPdfA;

        NFE_ASSERT(IsValid(outResult.directPdfA) && outResult.directPdfA >= 0.0f, "");
        NFE_ASSERT(IsValid(outResult.emissionPdfW) && outResult.emissionPdfW >= 0.0f, "");

        color *= RayColor::ResolveRGB(param.wavelength, textureColor);
    }
    else
    {
        // generate random direction on sphere
        outResult.direction = SamplingHelpers::GetSphere(param.directionSample);
        outResult.directPdfA = UniformSpherePdf();
        outResult.emissionPdfW = UniformSpherePdf();

        // evaluate texture
        if (mTexture)
        {
            const Vec4f coords = CartesianToSpherical(-outResult.direction);
            NFE_ASSERT(coords.IsValid(), "");

            const Vec4f textureColor = Vec4f::Max(Vec4f::Zero(), mTexture->Evaluate(coords));
            NFE_ASSERT(textureColor.IsValid(), "");

            color *= RayColor::ResolveRGB(param.wavelength, textureColor);
        }
    }

    // generate random origin
    const Vec4f uv = SamplingHelpers::GetDisk(param.positionSample);
    {
        Vec4f u, v;
        BuildOrthonormalBasis(outResult.direction, u, v);

        outResult.emissionPdfW *= UniformCirclePdf(SceneRadius);
        outResult.position = SceneRadius * (u * uv.x + v * uv.y - outResult.direction);
    }

    outResult.cosAtLight = 1.0f;

    return color;
}

ILight::Flags BackgroundLight::GetFlags() const
{
    return Flag_None;
}

} // namespace RT
} // namespace NFE

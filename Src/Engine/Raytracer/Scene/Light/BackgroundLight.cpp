#include "PCH.h"
#include "BackgroundLight.h"
#include "../../Rendering/RenderingContext.h"
#include "../../Rendering/ShadingData.h"
#include "../../Textures/Texture.h"
#include "../Camera.h"
#include "../../../Common/Math/Transcendental.hpp"
#include "../../../Common/Math/Geometry.hpp"
#include "../../../Common/Math/SamplingHelpers.hpp"
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
static const float SceneRadius = 30.0f; // TODO

BackgroundLight::BackgroundLight() = default;

BackgroundLight::BackgroundLight(const Math::HdrColorRGB& color)
    : ILight(color)
{}

const Box BackgroundLight::GetBoundingBox() const
{
    return Box::Full();
}

const RayColor BackgroundLight::GetBackgroundColor(const Vec4f& dir, const Wavelength& wavelength) const
{
    NFE_UNUSED(dir);

    RayColor color = GetColor()->Resolve(wavelength);

    // sample environment map
    if (mTexture)
    {
        const Vec4f coords = CartesianToSphericalCoordinates(dir);
        NFE_ASSERT(coords.IsValid());

        const Vec4f textureColor = Vec4f::Max(Vec4f::Zero(), mTexture->Evaluate(coords));

        color *= RayColor::ResolveRGB(wavelength, textureColor);
    }

    return color;
}

const RayColor BackgroundLight::Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const
{
    const Vec4f randomDirLocalSpace = SamplingHelpers::GetHemishpere(param.sample);
    outResult.directionToLight = param.intersection.LocalToWorld(randomDirLocalSpace);
    outResult.directPdfW = UniformHemispherePdf();
    outResult.emissionPdfW = UniformSpherePdf() * UniformCirclePdf(SceneRadius);
    outResult.distance = BackgroundLightDistance;
    outResult.cosAtLight = 1.0f;

    // TODO include light rotation
    return GetBackgroundColor(outResult.directionToLight, param.wavelength);
}

const RayColor BackgroundLight::GetRadiance(const RadianceParam& param, float* outDirectPdfA, float* outEmissionPdfW) const
{
    if (outDirectPdfA)
    {
        *outDirectPdfA = UniformHemispherePdf();
    }

    if (outEmissionPdfW)
    {
        *outEmissionPdfW = UniformSpherePdf() * UniformCirclePdf(SceneRadius);
    }

    // TODO include light rotation
    return GetBackgroundColor(param.ray.dir, param.context.wavelength);
}

const RayColor BackgroundLight::Emit(const EmitParam& param, EmitResult& outResult) const
{
    // generate random direction on sphere
    // TODO texture importance sampling
    outResult.direction = SamplingHelpers::GetSphere(param.directionSample);

    // generate random origin
    const Vec4f uv = SamplingHelpers::GetCircle(param.positionSample);
    {
        Vec4f u, v;
        BuildOrthonormalBasis(outResult.direction, u, v);

        outResult.position = SceneRadius * (u * uv.x + v * uv.y - outResult.direction);
    }

    outResult.directPdfA = UniformHemispherePdf();
    outResult.emissionPdfW = UniformSpherePdf() * UniformCirclePdf(SceneRadius);
    outResult.cosAtLight = 1.0f;

    // TODO include light rotation
    return GetBackgroundColor(-outResult.direction, param.wavelength);
}

ILight::Flags BackgroundLight::GetFlags() const
{
    return Flag_None;
}

} // namespace RT
} // namespace NFE

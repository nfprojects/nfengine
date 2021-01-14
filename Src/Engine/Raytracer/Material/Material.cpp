#include "PCH.h"
#include "Material.h"
#include "../Common/Math/HdrColor.hpp"
#include "../Common/Math/Utils.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_CLASS(NFE::RT::DispersionParams)
{
    NFE_CLASS_MEMBER(enable);
    NFE_CLASS_MEMBER(C);
    NFE_CLASS_MEMBER(D);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_CLASS(NFE::RT::Material)
{
    NFE_CLASS_MEMBER(mBSDF).Name("BSDF").NonNull();
    NFE_CLASS_MEMBER(emission);
    NFE_CLASS_MEMBER(baseColor);
    NFE_CLASS_MEMBER(roughness);
    NFE_CLASS_MEMBER(roughnessAnisotropy);
    NFE_CLASS_MEMBER(metalness);
    NFE_CLASS_MEMBER(IoR).Name("Index of Refraction").Min(0.0f).Max(4.0f);
    NFE_CLASS_MEMBER(K).Name("Extinction coefficient").Min(0.0f).Max(10.0f);
    NFE_CLASS_MEMBER(normalMapStrength).Min(0.0f).Max(5.0f);
    NFE_CLASS_MEMBER(dispersion);
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

const char* Material::DefaultBsdfName = "diffuse";

DispersionParams::DispersionParams()
{
    enable = false;
    // BK7 glass
    C = 0.00420f;
    D = 0.0f;
}

Material::Material(const char* debugName)
    : debugName(debugName)
    , baseColor(Math::HdrColorRGB(0.7f, 0.7f, 0.7f))
{
    SetBsdf(Material::DefaultBsdfName);
    Compile();
}

MaterialPtr Material::Create()
{
    return MakeSharedPtr<Material>();
}

void Material::SetBsdf(const StringView& bsdfName)
{
    // TEMP total hack for testing
    if (bsdfName == "metal" || bsdfName == "roughMetal")
    {
        const uint32 lutSize = 15;

        const float wavelengths[lutSize] =
        {
            0.3679f,
            0.3815f,
            0.3974f,
            0.4133f,
            0.4305f,
            0.4509f,
            0.4714f,
            0.4959f,
            0.5209f,
            0.5486f,
            0.5821f,
            0.6168f,
            0.6595f,
            0.7045f,
            0.7560f,
        };

        // gold
        const Vec2f ior[lutSize] =
        {
            { 1.48f, 1.895f },
            { 1.46f, 1.933f },
            { 1.47f, 1.952f },
            { 1.46f, 1.958f },
            { 1.45f, 1.948f },
            { 1.38f, 1.914f },
            { 1.31f, 1.849f },
            { 1.04f, 1.833f },
            { 0.62f, 2.081f },
            { 0.43f, 2.455f },
            { 0.29f, 2.863f },
            { 0.21f, 3.272f },
            { 0.14f, 3.697f },
            { 0.13f, 4.103f },
            { 0.14f, 4.542f },
        };

        // copper
        /*
        const float wavelengths[lutSize] =
        {
            0.3679f,
            0.3815f,
            0.3974f,
            0.4133f,
            0.4305f,
            0.4509f,
            0.4714f,
            0.4959f,
            0.5209f,
            0.5486f,
            0.5821f,
            0.6168f,
            0.6595f,
            0.7045f,
            0.7560f,
        };
        
        const Vec2f ior[lutSize] =
        {
            { 1.36f, 1.975f },
            { 1.33f, 2.045f },
            { 1.32f, 2.116f },
            { 1.28f, 2.207f },
            { 1.25f, 2.305f },
            { 1.24f, 2.397f },
            { 1.25f, 2.483f },
            { 1.22f, 2.564f },
            { 1.18f, 2.608f },
            { 1.02f, 2.577f },
            { 0.70f, 2.704f },
            { 0.30f, 3.205f },
            { 0.22f, 3.747f },
            { 0.21f, 4.205f },
            { 0.24f, 4.665f },
        };
        */

        mIorLookupTable.Init(ArrayView<const float>(wavelengths, lutSize), ArrayView<const Vec2f>(ior, lutSize));
    }

    DynArray<const RTTI::ClassType*> types;
    RTTI::GetType<BSDF>()->ListSubtypes(types);

    for (const RTTI::ClassType* type : types)
    {
        const BSDF* defaultObject = type->GetDefaultObject<BSDF>();
        if (type->IsConstructible())
        {
            if ((type->GetName() == bsdfName) || (defaultObject && (defaultObject->GetShortName() == bsdfName)))
            {
                mBSDF = UniquePtr<BSDF>(type->CreateObject<BSDF>());
                return;
            }
        }
    }

    NFE_LOG_ERROR("Unknown BSDF name: '%.*s'", bsdfName.Length(), bsdfName.Data());
}

static UniquePtr<Material> CreateDefaultMaterial()
{
    return MakeUniquePtr<Material>("default");
}

const MaterialPtr& Material::GetDefaultMaterial()
{
    static MaterialPtr sDefaultMaterial = CreateDefaultMaterial();
    return sDefaultMaterial;
}

Material::~Material() = default;

Material::Material(Material&&) = default;
Material& Material::operator = (Material&&) = default;

void Material::Compile()
{
    NFE_ASSERT(emission.IsValid(), "");
    NFE_ASSERT(baseColor.IsValid(), "");
    NFE_ASSERT(IsValid(roughness.baseValue), "");
    NFE_ASSERT(IsValid(roughnessAnisotropy.baseValue), "");
    NFE_ASSERT(IsValid(metalness.baseValue), "");
    NFE_ASSERT(IsValid(normalMapStrength) && normalMapStrength >= 0.0f, "");
    NFE_ASSERT(IsValid(IoR) && IoR >= 0.0f, "");
    NFE_ASSERT(IsValid(K) && K >= 0.0f, "");
}

const Vec4f Material::GetNormalVector(const Vec4f& uv) const
{
    const Vec4f z = VECTOR_Z;

    Vec4f normal = z;

    if (normalMap)
    {
        normal = normalMap->Evaluate(uv);

        // scale from [0...1] to [-1...1]
        normal = UnipolarToBipolar(normal);

        // reconstruct Z
        normal.z = Sqrt(Max(0.0f, 1.0f - normal.SqrLength2()));

        normal = Vec4f::Lerp(z, normal, normalMapStrength);
    }

    return normal;
}

bool Material::GetMaskValue(const Vec4f& uv) const
{
    if (maskMap)
    {
        const float maskTreshold = 0.5f;
        return maskMap->Evaluate(uv).x > maskTreshold;
    }

    return true;
}

const RayColor Material::EvaluateMetalFresnel(float NdotV, const Wavelength& wavelength) const
{
#ifdef NFE_ENABLE_SPECTRAL_RENDERING

    RayColor F = RayColor::Zero();
    const auto wavelengthInMeters = wavelength.GetInMeters();

    for (uint32 i = 0; i < Wavelength::NumComponents; ++i)
    {
        const Vec2f ior = mIorLookupTable.Sample(wavelengthInMeters[i] * 1.0e+6f);
        F.value[i] = FresnelMetal(NdotV, ior.x, ior.y);
    }

    return F;

#else // !NFE_ENABLE_SPECTRAL_RENDERING

    NFE_UNUSED(wavelength);

    // TODO
    // This is wrong!
    // IoR depends on the wavelength and this is the source of metal color.
    // Metal always reflect 100% pure white at grazing angle.
    return RayColor(FresnelMetal(NdotV, IoR, K));

#endif // NFE_ENABLE_SPECTRAL_RENDERING
}

void Material::EvaluateShadingData(const Wavelength& wavelength, ShadingData& shadingData) const
{
    shadingData.materialParams.baseColor = baseColor.Evaluate(shadingData.intersection.texCoord, wavelength);
    shadingData.materialParams.emissionColor = emission.Evaluate(shadingData.intersection.texCoord, wavelength);
    shadingData.materialParams.roughness = roughness.Evaluate(shadingData.intersection.texCoord);
    shadingData.materialParams.roughnessAnisotropy = roughnessAnisotropy.Evaluate(shadingData.intersection.texCoord);
    shadingData.materialParams.metalness = metalness.Evaluate(shadingData.intersection.texCoord);
    shadingData.materialParams.IoR = IoR;
}

const RayColor Material::Evaluate(
    ISampler& sampler,
    const Wavelength& wavelength,
    const ShadingData& shadingData,
    const Vec4f& incomingDirWorldSpace,
    float* outPdfW, float* outReversePdfW) const
{
    if (!mBSDF)
    {
        return RayColor::Zero();
    }

    const Vec4f outgoingDirLocalSpace = shadingData.intersection.WorldToLocal(shadingData.outgoingDirWorldSpace);
    const Vec4f incomingDirLocalSpace = shadingData.intersection.WorldToLocal(incomingDirWorldSpace);

    const BSDF::EvaluationContext evalContext =
    {
        *this,
        shadingData.materialParams,
        sampler,
        wavelength,
        outgoingDirLocalSpace,
        incomingDirLocalSpace
    };

    return mBSDF->Evaluate(evalContext, outPdfW, outReversePdfW);
}

const RayColor Material::Sample(
    Wavelength& wavelength,
    Vec4f& outIncomingDirWorldSpace,
    const ShadingData& shadingData,
    ISampler& sampler,
    float* outPdfW,
    BSDF::EventType* outSampledEvent) const
{
    BSDF::SamplingContext samplingContext =
    {
        *this,
        shadingData.materialParams,
        sampler,
        shadingData.intersection.WorldToLocal(shadingData.outgoingDirWorldSpace),
        wavelength,
    };

    // BSDF sampling (in local space)
    // TODO don't compute PDF if not requested
    if (!mBSDF || !mBSDF->Sample(samplingContext))
    {
        if (outSampledEvent)
        {
            *outSampledEvent = BSDF::NullEvent;
        }

        return RayColor::Zero();
    }

    NFE_ASSERT(IsValid(samplingContext.outPdf), "");
    NFE_ASSERT(samplingContext.outPdf >= 0.0f, "");
    NFE_ASSERT(samplingContext.outIncomingDir.IsValid(), "");
    NFE_ASSERT(samplingContext.outColor.IsValid(), "");

    // convert incoming light direction back to world space
    outIncomingDirWorldSpace = shadingData.intersection.LocalToWorld(samplingContext.outIncomingDir);

    if (outPdfW)
    {
        *outPdfW = samplingContext.outPdf;
    }

    if (outSampledEvent)
    {
        *outSampledEvent = samplingContext.outEventType;
    }

    return samplingContext.outColor;
}


} // namespace RT
} // namespace NFE

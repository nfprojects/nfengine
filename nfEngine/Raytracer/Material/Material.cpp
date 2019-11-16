#include "PCH.h"
#include "Material.h"
#include "BSDF/BSDF.h"
#include "Color/Spectrum.h"
#include "../nfCommon/Logger/Logger.hpp"


NFE_BEGIN_DEFINE_CLASS(NFE::RT::DispersionParams)
{
    NFE_CLASS_MEMBER(enable);
    NFE_CLASS_MEMBER(C);
    NFE_CLASS_MEMBER(D);
}
NFE_END_DEFINE_CLASS()

NFE_BEGIN_DEFINE_CLASS(NFE::RT::Material)
{
    NFE_CLASS_MEMBER(mBSDF).Name("BSDF").NonNull();
    NFE_CLASS_MEMBER(emission);
    NFE_CLASS_MEMBER(baseColor);
    NFE_CLASS_MEMBER(roughness);
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
{
}

MaterialPtr Material::Create()
{
    return MaterialPtr(new Material);
}

void Material::SetBsdf(const String& bsdfName)
{
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

    NFE_LOG_ERROR("Unknown BSDF name: '%s'", bsdfName.Str());
}

static UniquePtr<Material> CreateDefaultMaterial()
{
    UniquePtr<Material> material = MakeUniquePtr<Material>("default");
    material->SetBsdf(Material::DefaultBsdfName);
    material->Compile();
    return material;
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
    NFE_ASSERT(emission.baseValue.IsValid());
    NFE_ASSERT(baseColor.baseValue.IsValid());
    NFE_ASSERT(IsValid(roughness.baseValue));
    NFE_ASSERT(IsValid(metalness.baseValue));
    NFE_ASSERT(IsValid(normalMapStrength) && normalMapStrength >= 0.0f);
    NFE_ASSERT(IsValid(IoR) && IoR >= 0.0f);
    NFE_ASSERT(IsValid(K) && K >= 0.0f);
}

const Vector4 Material::GetNormalVector(const Vector4& uv) const
{
    const Vector4 z = VECTOR_Z;

    Vector4 normal = z;

    if (normalMap)
    {
        normal = normalMap->Evaluate(uv);

        // scale from [0...1] to [-1...1]
        normal = UnipolarToBipolar(normal);

        // reconstruct Z
        normal.z = Sqrt(Max(0.0f, 1.0f - normal.SqrLength2()));

        normal = Vector4::Lerp(z, normal, normalMapStrength);
    }

    return normal;
}

bool Material::GetMaskValue(const Vector4& uv) const
{
    if (maskMap)
    {
        const float maskTreshold = 0.5f;
        return maskMap->Evaluate(uv).x > maskTreshold;
    }

    return true;
}

void Material::EvaluateShadingData(const Wavelength& wavelength, ShadingData& shadingData) const
{
    shadingData.materialParams.baseColor = RayColor::Resolve(wavelength, Spectrum(baseColor.Evaluate(shadingData.intersection.texCoord)));
    shadingData.materialParams.emissionColor = RayColor::Resolve(wavelength, Spectrum(emission.Evaluate(shadingData.intersection.texCoord)));
    shadingData.materialParams.roughness = roughness.Evaluate(shadingData.intersection.texCoord);
    shadingData.materialParams.metalness = metalness.Evaluate(shadingData.intersection.texCoord);
    shadingData.materialParams.IoR = IoR;
}

const RayColor Material::Evaluate(
    const Wavelength& wavelength,
    const ShadingData& shadingData,
    const Vector4& incomingDirWorldSpace,
    float* outPdfW, float* outReversePdfW) const
{
    if (!mBSDF)
    {
        return RayColor::Zero();
    }

    const Vector4 outgoingDirLocalSpace = shadingData.intersection.WorldToLocal(shadingData.outgoingDirWorldSpace);
    const Vector4 incomingDirLocalSpace = shadingData.intersection.WorldToLocal(incomingDirWorldSpace);

    const BSDF::EvaluationContext evalContext =
    {
        *this,
        shadingData.materialParams,
        wavelength,
        outgoingDirLocalSpace,
        incomingDirLocalSpace
    };

    return mBSDF->Evaluate(evalContext, outPdfW, outReversePdfW);
}

const RayColor Material::Sample(
    Wavelength& wavelength,
    Vector4& outIncomingDirWorldSpace,
    const ShadingData& shadingData,
    const Float3& sample,
    float* outPdfW,
    BSDF::EventType* outSampledEvent) const
{
    BSDF::SamplingContext samplingContext =
    {
        *this,
        shadingData.materialParams,
        sample,
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

    NFE_ASSERT(IsValid(samplingContext.outPdf));
    NFE_ASSERT(samplingContext.outPdf >= 0.0f);
    NFE_ASSERT(samplingContext.outIncomingDir.IsValid());
    NFE_ASSERT(samplingContext.outColor.IsValid());

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

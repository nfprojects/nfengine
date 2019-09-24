#include "PCH.h"
#include "Material.h"

#include "BSDF/NullBSDF.h"
#include "BSDF/DiffuseBSDF.h"
#include "BSDF/RoughDiffuseBSDF.h"
#include "BSDF/DielectricBSDF.h"
#include "BSDF/RoughDielectricBSDF.h"
#include "BSDF/MetalBSDF.h"
#include "BSDF/RoughMetalBSDF.h"
#include "BSDF/PlasticBSDF.h"
#include "BSDF/RoughPlasticBSDF.h"

#include "Color/Spectrum.h"

#include "../nfCommon/Logger/Logger.hpp"

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

const char* Material::DefaultBsdfName = "diffuse";

DispersionParams::DispersionParams()
{
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
    // TODO use some kind of reflection for this
    if (bsdfName == "null")
    {
        mBSDF = MakeUniquePtr<NullBSDF>();
    }
    else if (bsdfName == "diffuse")
    {
        mBSDF = MakeUniquePtr<DiffuseBSDF>();
    }
    else if (bsdfName == "roughDiffuse")
    {
        mBSDF = MakeUniquePtr<RoughDiffuseBSDF>();
    }
    else if (bsdfName == "dielectric")
    {
        mBSDF = MakeUniquePtr<DielectricBSDF>();
    }
    else if (bsdfName == "roughDielectric")
    {
        mBSDF = MakeUniquePtr<RoughDielectricBSDF>();
    }
    else if (bsdfName == "metal")
    {
        mBSDF = MakeUniquePtr<MetalBSDF>();
    }
    else if (bsdfName == "roughMetal")
    {
        mBSDF = MakeUniquePtr<RoughMetalBSDF>();
    }
    else if (bsdfName == "plastic")
    {
        mBSDF = MakeUniquePtr<PlasticBSDF>();
    }
    else if (bsdfName == "roughPlastic")
    {
        mBSDF = MakeUniquePtr<RoughPlasticBSDF>();
    }
    else
    {
        NFE_LOG_ERROR("Unknown BSDF name: '%s'", bsdfName.Str());
    }
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

Material::~Material()
{
}

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

    emission.baseValue = Vector4::Max(Vector4::Zero(), emission.baseValue);
    baseColor.baseValue = Vector4::Max(Vector4::Zero(), Vector4::Min(VECTOR_ONE, baseColor.baseValue));
}

const Vector4 Material::GetNormalVector(const Vector4& uv) const
{
    Vector4 normal(0.0f, 0.0f, 1.0f, 0.0f);

    if (normalMap)
    {
        normal = normalMap->Evaluate(uv);

        // scale from [0...1] to [-1...1]
        normal = UnipolarToBipolar(normal);

        // reconstruct Z
        normal.z = Sqrt(Max(0.0f, 1.0f - normal.SqrLength2()));

        normal = Vector4::Lerp(VECTOR_Z, normal, normalMapStrength);
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
    NFE_ASSERT(mBSDF, "Material must have a BSDF assigned");

    const Vector4 incomingDirLocalSpace = shadingData.intersection.WorldToLocal(incomingDirWorldSpace);

    const BSDF::EvaluationContext evalContext =
    {
        *this,
        shadingData.materialParams,
        wavelength,
        shadingData.intersection.WorldToLocal(shadingData.outgoingDirWorldSpace),
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
    NFE_ASSERT(mBSDF, "Material must have a BSDF assigned");

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
    if (!mBSDF->Sample(samplingContext))
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

///


} // namespace RT
} // namespace NFE

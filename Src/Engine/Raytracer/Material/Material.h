#pragma once

#include "BSDF/BSDF.h"
#include "MaterialParameter.h"
#include "../Textures/Texture.h"
#include "../Utils/Memory.h"
#include "../Utils/LookupTable.h"
#include "../Color/RayColor.h"
#include "../../Common/Math/Ray.hpp"
#include "../../Common/Containers/String.hpp"
#include "../../Common/Containers/SharedPtr.hpp"
#include "../../Common/Memory/Aligned.hpp"
#include "../../Common/Reflection/ReflectionClassDeclare.hpp"
#include "../../Common/Reflection/Types/ReflectionUniquePtrType.hpp"

namespace NFE {
namespace RT {

// coefficients of Sellmeier dispersion equation
class NFE_RAYTRACER_API DispersionParams
{
    NFE_DECLARE_CLASS(DispersionParams)

public:
    bool enable;
    float C;
    float D;

    DispersionParams();
};

using MaterialPtr = Common::SharedPtr<Material>;

// simple PBR material
class NFE_ALIGN(16) Material
{
    NFE_DECLARE_CLASS(Material)

public:
    NFE_ALIGNED_CLASS(16)

    NFE_RAYTRACER_API Material(const char* debugName = "<unnamed>");
    NFE_RAYTRACER_API ~Material();
    NFE_RAYTRACER_API Material(Material&&);
    NFE_RAYTRACER_API Material& operator = (Material&&);

    NFE_RAYTRACER_API static const char* DefaultBsdfName;

    NFE_RAYTRACER_API static MaterialPtr Create();

    static const MaterialPtr& GetDefaultMaterial();

    Common::String debugName;

    // light emitted by the material itself
    // useful for lamps, etc
    ColorMaterialParameter emission;

    // a.k.a. albedo
    // for metals this is specular/reflection color
    // for dielectrics this is diffuse color
    ColorMaterialParameter baseColor;

    // 0.0 - smooth, perfect mirror
    // 1.0 - rough, maximum diffusion
    MaterialParameter roughness = 0.1f;

    // 0.0 - pure X anisotropy
    // 0.5 - isotropic roughness
    // 1.0 - pure Y anisotropy
    MaterialParameter roughnessAnisotropy = 0.5f;

    // TODO move to "Principled BSDF"
    // blends between dielectric/metal models
    MaterialParameter metalness = 0.0f;

    // normal map lerp value
    float normalMapStrength = 1.0f;

    // index of refraction (real and imaginary parts)
    float IoR = 1.5f; // NOTE: not used when material is dispersive
    float K = 4.0f;

    // wavelength-dependent IoR (for physically-correct spectral rendering of metals)
    LookupTable<Math::Vec2f> mIorLookupTable;

    // chromatic dispersion parameters
    DispersionParams dispersion;

    // textures
    TexturePtr maskMap = nullptr;
    TexturePtr normalMap = nullptr;

    // TODO material layers

    NFE_RAYTRACER_API void SetBsdf(const Common::StringView& bsdfName);

    NFE_FORCE_INLINE const BSDF* GetBSDF() const { return mBSDF.Get(); }

    NFE_RAYTRACER_API void Compile();

    const Math::Vec4f GetNormalVector(const Math::Vec4f& uv) const;
    bool GetMaskValue(const Math::Vec4f& uv) const;

    const RayColor EvaluateMetalFresnel(float NdotV, const Wavelength& wavelength) const;

    void EvaluateShadingData(const Wavelength& wavelength, ShadingData& shadingData) const;

    // sample material's BSDFs
    const RayColor Sample(
        Wavelength& wavelength,
        Math::Vec4f& outIncomingDirWorldSpace,
        const ShadingData& shadingData,
        ISampler& sampler,
        float* outPdfW = nullptr,
        BSDF::EventType* outSampledEvent = nullptr) const;

    // calculate amount of light reflected from incoming direction to outgoing direction
    const RayColor Evaluate(
        ISampler& sampler,
        const Wavelength& wavelength,
        const ShadingData& shadingData,
        const Math::Vec4f& incomingDirWorldSpace,
        float* outPdfW = nullptr,
        float* outReversePdfW = nullptr) const;

private:
    Material(const Material&) = delete;
    Material& operator = (const Material&) = delete;

    Common::UniquePtr<BSDF> mBSDF;
};

} // namespace RT
} // namespace NFE

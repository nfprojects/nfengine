#pragma once

#include "BSDF/BSDF.h"
#include "MaterialParameter.h"
#include "../Textures/Texture.h"
#include "../Utils/Memory.h"
#include "../Color/RayColor.h"
#include "../../nfCommon/Math/Ray.hpp"
#include "../../nfCommon/Containers/String.hpp"
#include "../../nfCommon/Containers/SharedPtr.hpp"
#include "../../nfCommon/Memory/Aligned.hpp"
#include "../../nfCommon/Reflection/ReflectionClassMacros.hpp"
#include "../../nfCommon/Reflection/Types/ReflectionUniquePtrType.hpp"

namespace NFE {
namespace RT {

// coefficients of Sellmeier dispersion equation
class NFE_RAYTRACER_API DispersionParams
{
    NFE_DECLARE_CLASS(DispersionParams);

public:
    bool enable;
    float C;
    float D;

    DispersionParams();
};

using MaterialPtr = Common::SharedPtr<Material>;

// simple PBR material
class NFE_ALIGN(16) Material : public Common::Aligned<16>
{
    NFE_DECLARE_CLASS(Material);

public:
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
    ColorMaterialParameter baseColor = Math::HdrColorRGB(0.7f, 0.7f, 0.7f);

    // 0.0 - smooth, perfect mirror
    // 1.0 - rough, maximum diffusion
    MaterialParameter roughness = 0.1f;

    // TODO move to "Principled BSDF"
    // blends between dielectric/metal models
    MaterialParameter metalness = 0.0f;

    // normal map lerp value
    float normalMapStrength = 1.0f;

    // index of refraction (real and imaginary parts)
    float IoR = 1.5f; // NOTE: not used when material is dispersive
    float K = 4.0f;

    // chromatic dispersion parameters
    DispersionParams dispersion;

    // textures
    TexturePtr maskMap = nullptr;
    TexturePtr normalMap = nullptr;

    // TODO material layers

    NFE_RAYTRACER_API void SetBsdf(const Common::String& bsdfName);

    NFE_FORCE_INLINE const BSDF* GetBSDF() const { return mBSDF.Get(); }

    NFE_RAYTRACER_API void Compile();

    const Math::Vector4 GetNormalVector(const Math::Vector4& uv) const;
    bool GetMaskValue(const Math::Vector4& uv) const;

    void EvaluateShadingData(const Wavelength& wavelength, ShadingData& shadingData) const;

    // sample material's BSDFs
    const RayColor Sample(
        Wavelength& wavelength,
        Math::Vector4& outIncomingDirWorldSpace,
        const ShadingData& shadingData,
        const Math::Float3& sample,
        float* outPdfW = nullptr,
        BSDF::EventType* outSampledEvent = nullptr) const;

    // calculate amount of light reflected from incoming direction to outgoing direction
    const RayColor Evaluate(
        const Wavelength& wavelength,
        const ShadingData& shadingData,
        const Math::Vector4& incomingDirWorldSpace,
        float* outPdfW = nullptr,
        float* outReversePdfW = nullptr) const;

private:
    Material(const Material&) = delete;
    Material& operator = (const Material&) = delete;

    Common::UniquePtr<BSDF> mBSDF;
};

} // namespace RT
} // namespace NFE

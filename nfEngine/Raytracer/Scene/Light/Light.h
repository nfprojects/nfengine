#pragma once

#include "../../Raytracer.h"
#include "../../Color/RayColor.h"
#include "../../Color/Color.h"
#include "../../Utils/Memory.h"
#include "../../../nfCommon/Math/Box.hpp"
#include "../../../nfCommon/Math/Matrix4.hpp"
#include "../../../nfCommon/Math/HdrColor.hpp"
#include "../../../nfCommon/Containers/UniquePtr.hpp"
#include "../../../nfCommon/Memory/Aligned.hpp"
#include "../../../nfCommon/Reflection/ReflectionClassDeclare.hpp"
#include "../../../nfCommon/Reflection/Object.hpp"

namespace NFE {
namespace RT {

// abstract light
class NFE_ALIGN(16) ILight 
    : public Common::Aligned<16>
    , public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ILight);

public:
    static constexpr const float BackgroundLightDistance = std::numeric_limits<float>::max();
    static constexpr const float CosEpsilon = 0.9999f;

    enum Flags : uint8
    {
        Flag_None       = 0,
        Flag_IsFinite   = 1 << 0,   // light has finite extent (e.g. point or area light)
        Flag_IsDelta    = 1 << 1,   // light cannot be hit by camera ray directly (e.g. directional light or point light)
    };

    struct RadianceParam
    {
        RenderingContext& context;
        const Math::Ray& ray;
        const Math::Vector4 hitPoint = Math::Vector4::Zero();
        const float cosAtLight = 1.0f;
        bool rendererSupportsSolidAngleSampling = true;
    };

    struct IlluminateParam
    {
        const Math::Matrix4 worldToLight;       // transform from world space to light local space
        const Math::Matrix4 lightToWorld;       // transform from light local space to world space
        const IntersectionData& intersection;   // intersection data of the shaded object
        Wavelength& wavelength;
        Math::Float3 sample;
        bool rendererSupportsSolidAngleSampling = true;
    };

    struct IlluminateResult
    {
        Math::Vector4 directionToLight = Math::Vector4::Zero();
        float distance = -1.0f;
        float directPdfW = -1.0f;
        float emissionPdfW = -1.0f;
        float cosAtLight = -1.0f;
    };

    struct EmitParam
    {
        const Math::Matrix4 lightToWorld; // transform from light local space to world space
        Wavelength& wavelength;
        Math::Float3 positionSample;
        Math::Float2 directionSample;
    };

    struct EmitResult
    {
        Math::Vector4 position;
        Math::Vector4 direction;
        float directPdfA;
        float emissionPdfW;
        float cosAtLight;
    };

    explicit ILight(const Math::HdrColorRGB& color = Math::HdrColorRGB(1.0f));
    NFE_RAYTRACER_API virtual ~ILight() = default;

    NFE_FORCE_INLINE const IColor* GetColor() const { return mColor.Get(); }
    NFE_RAYTRACER_API void SetColor(const ColorPtr& color);

    // get light's surface bounding box
    virtual const Math::Box GetBoundingBox() const = 0;

    // check if a ray hits the light
    virtual void Traverse(const SingleTraversalContext& context, const uint32 objectID) const;
    virtual bool Traverse_Shadow(const SingleTraversalContext& context) const;

    // Illuminate a point in the scene.
    // Returns probability of sampling the returned direction.
    virtual const RayColor Illuminate(const IlluminateParam& param, IlluminateResult& outResult) const = 0;

    // Emit random light photon from the light
    virtual const RayColor Emit(const EmitParam& param, EmitResult& outResult) const = 0;

    // Returns radiance for ray hitting the light directly
    // Optionally returns probability of hitting this point and emitting a photon in that direction
    virtual const RayColor GetRadiance(const RadianceParam& param, float* outDirectPdfA = nullptr, float* outEmissionPdfW = nullptr) const;

    // Get light flags.
    virtual Flags GetFlags() const = 0;

private:
    // light object cannot be copied
    ILight(const ILight&) = delete;
    ILight& operator = (const ILight&) = delete;

    ColorPtr mColor;
};

using LightPtr = Common::UniquePtr<ILight>;

} // namespace RT
} // namespace NFE

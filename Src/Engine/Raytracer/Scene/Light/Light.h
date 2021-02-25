#pragma once

#include "../../Raytracer.h"
#include "../../Color/RayColor.h"
#include "../../Color/Color.h"
#include "../../Utils/Memory.h"
#include "../../../Common/Math/Box.hpp"
#include "../../../Common/Math/Matrix4.hpp"
#include "../../../Common/Math/HdrColor.hpp"
#include "../../../Common/Containers/UniquePtr.hpp"
#include "../../../Common/Memory/Aligned.hpp"
#include "../../../Common/Reflection/ReflectionClassDeclare.hpp"
#include "../../../Common/Reflection/Object.hpp"

namespace NFE {
namespace RT {

// abstract light
class NFE_ALIGN(16) ILight : public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ILight)

public:
    NFE_ALIGNED_CLASS(16)

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
        const Math::Vec4f hitPoint = Math::Vec4f::Zero();
        const float cosAtLight = 1.0f;
        bool rendererSupportsSolidAngleSampling = true;
    };

    struct IlluminateParam
    {
        const Math::Matrix4 worldToLight;       // transform from world space to light local space
        const Math::Matrix4 lightToWorld;       // transform from light local space to world space
        const IntersectionData& intersection;   // intersection data of the shaded object
        Wavelength& wavelength;
        Math::Vec3f sample;
        bool rendererSupportsSolidAngleSampling = true;
    };

    struct IlluminateResult
    {
        Math::Vec4f directionToLight = Math::Vec4f::Zero();
        float distance = -1.0f;
        float directPdfW = -1.0f;
        float emissionPdfW = -1.0f;
        float cosAtLight = -1.0f;
    };

    struct EmitParam
    {
        const Math::Matrix4 lightToWorld; // transform from light local space to world space
        Wavelength& wavelength;
        Math::Vec3f positionSample;
        Math::Vec2f directionSample;
    };

    struct EmitResult
    {
        Math::Vec4f position;
        Math::Vec4f direction;
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
    virtual bool Traverse_Shadow(const SingleTraversalContext& context, const uint32 objectID) const;

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

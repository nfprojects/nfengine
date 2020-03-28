#pragma once

#include "../Raytracer.h"

#include "../../Common/Math/Transform.hpp"
#include "../../Common/Math/Ray.hpp"
#include "../../Common/Math/Random.hpp"
#include "../../Common/Math/Simd8Ray.hpp"
#include "../../Common/Math/Matrix4.hpp"
#include "../../Common/Containers/SharedPtr.hpp"

namespace NFE {
namespace RT {

using TexturePtr = Common::SharedPtr<ITexture>;

enum class BokehShape : uint8
{
    Circle = 0,
    Hexagon,
    Square,
    NGon,
    Texture,
};

/**
 * Depth of Field settings.
 */
struct DOFSettings
{
    // distance from camera at which plane of perfect focus is located
    float focalPlaneDistance = 2.0f;

    // the bigger value, the bigger out-of-focus blur
    float aperture = 0.1f;

    bool enable = false;

    BokehShape bokehShape = BokehShape::Circle;

    // used when bokeh type is "NGon"
    uint32 apertureBlades = 5;

    // used when bokeh type is "Texture"
    TexturePtr bokehTexture;
};


/**
 * Class describing camera for scene raytracing.
 */
class NFE_ALIGN(32) Camera
{
public:
    NFE_RAYTRACER_API Camera();

    NFE_RAYTRACER_API void SetTransform(const Math::Transform& transform);

    NFE_RAYTRACER_API void SetPerspective(float aspectRatio, float FoV);

    NFE_RAYTRACER_API void SetAngularVelocity(const Math::Quaternion& quat);

    NFE_FORCE_INLINE const Math::Transform& GetTransform() const { return mTransform; }
    NFE_FORCE_INLINE const Math::Matrix4& GetLocalToWorld() const { return mLocalToWorld; }

    // Sample camera transfrom for given time point
    NFE_FORCE_INLINE const Math::Matrix4 SampleTransform(const float time) const;

    // Generate ray for the camera for a given time
    // x and y coordinates should be in [0.0f, 1.0f) range.
    NFE_RAYTRACER_API NFE_FORCE_NOINLINE const Math::Ray GenerateRay(const Math::Vec4f& coords, RenderingContext& context) const;
    NFE_FORCE_NOINLINE const Math::Ray_Simd8 GenerateRay_Simd8(const Math::Vec2x8f& coords, RenderingContext& context) const;

    NFE_FORCE_INLINE const Math::Vec4f GenerateBokeh(const Math::Vec3f sample) const;
    NFE_FORCE_INLINE const Math::Vec2x8f GenerateBokeh_Simd8(RenderingContext& context) const;

    // Convert world-space coordinates to film-space coordinates including camera projection (0...1 range)
    bool WorldToFilm(const Math::Vec4f& worldPosition, Math::Vec4f& outFilmCoords) const;

    float PdfW(const Math::Vec4f& direction) const;

    // camera placement
    Math::Transform mTransform;

    // width to height ratio
    float mAspectRatio;

    // in radians, vertical angle
    float mFieldOfView;

    // depth of field settings
    DOFSettings mDOF;

    // camera lens distortion (0.0 - no distortion)
    float barrelDistortionConstFactor;
    float barrelDistortionVariableFactor;
    bool enableBarellDistortion;

private:
    float mTanHalfFoV;

    Math::Matrix4 mLocalToWorld;
    Math::Matrix4 mWorldToScreen;
};

} // namespace RT
} // namespace NFE

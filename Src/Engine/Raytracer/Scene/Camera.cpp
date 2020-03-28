#include "PCH.h"
#include "Camera.h"
#include "Rendering/RenderingContext.h"
#include "Sampling/GenericSampler.h"
#include "Textures/Texture.h"
#include "../Common/Math/SamplingHelpers.hpp"

namespace NFE {
namespace RT {

using namespace Math;

Camera::Camera()
    : mAspectRatio(1.0f)
    , mFieldOfView(DegToRad(20.0f))
    , barrelDistortionConstFactor(0.01f)
    , barrelDistortionVariableFactor(0.0f)
    , enableBarellDistortion(false)
    , mLocalToWorld(Matrix4::Identity())
    , mWorldToScreen(Matrix4::Identity())
{ }

void Camera::SetTransform(const Math::Transform& transform)
{
    NFE_ASSERT(transform.IsValid());

    mTransform = transform;
    mLocalToWorld = transform.ToMatrix();
}

void Camera::SetPerspective(float aspectRatio, float FoV)
{
    NFE_ASSERT(IsValid(aspectRatio) && aspectRatio > 0.0f);
    NFE_ASSERT(IsValid(FoV) && aspectRatio > 0.0f && FoV < NFE_MATH_PI);

    mAspectRatio = aspectRatio;
    mFieldOfView = FoV;
    mTanHalfFoV = tanf(mFieldOfView * 0.5f);

    {
        Matrix4 worldToLocal = mLocalToWorld.FastInverseNoScale();
        worldToLocal[0].w = 0.0f;
        worldToLocal[1].w = 0.0f;
        worldToLocal[2].w = 0.0f;
        worldToLocal[3].w = 1.0f;

        const Matrix4 projection = Matrix4::MakePerspective(aspectRatio, FoV, 0.01f, 1000.0f);
        mWorldToScreen = mLocalToWorld.FastInverseNoScale() * projection;
    }
}

void Camera::SetAngularVelocity(const Math::Quaternion& quat)
{
    NFE_UNUSED(quat);

    // TODO
    //mAngularVelocity = quat.Normalized();
    //mAngularVelocityIsZero = Quaternion::AlmostEqual(mAngularVelocity, Quaternion::Identity());
    //NFE_ASSERT(mAngularVelocity.IsValid());
}

const Matrix4 Camera::SampleTransform(const float time) const
{
    NFE_UNUSED(time);

    return mLocalToWorld;

    // TODO
    //const Vec4f position = Vec4f::MulAndAdd(mLinearVelocity, time, mTransform.GetTranslation());
    //const Quaternion& rotation0 = mTransform.GetRotation();

    //if (mAngularVelocityIsZero)
    //{
    //    return Transform(position, rotation0);
    //}

    //const Quaternion rotation1 = rotation0 * mAngularVelocity;
    //const Quaternion rotation = Quaternion::Interpolate(rotation0, rotation1, time);
    //return Transform(position, rotation);
}

const Ray Camera::GenerateRay(const Vec4f& coords, RenderingContext& context) const
{
    const Matrix4 transform = SampleTransform(context.time);
    Vec4f offsetedCoords = UnipolarToBipolar(coords);

    // barrel distortion
    if (barrelDistortionVariableFactor)
    {
        Vec4f radius = Vec4f::Dot2V(offsetedCoords, offsetedCoords);
        radius *= (barrelDistortionConstFactor + barrelDistortionVariableFactor * context.randomGenerator.GetFloat());
        offsetedCoords = Vec4f::MulAndAdd(offsetedCoords, radius, offsetedCoords);
    }

    // calculate ray direction (ideal, without DoF)
    Vec4f origin = transform.GetTranslation();
    Vec4f direction = Vec4f::MulAndAdd(
        Vec4f::MulAndAdd(transform[0], offsetedCoords.x * mAspectRatio, transform[1] * offsetedCoords.y),
        mTanHalfFoV,
        transform[2]);

    // depth of field
    if (mDOF.enable)
    {
        const Vec4f focusPoint = Vec4f::MulAndAdd(direction, mDOF.focalPlaneDistance, origin);

        const Vec4f right = transform[0];
        const Vec4f up = transform[1];

        const Vec3f sample{ context.sampler.GetVec2f(), 0.0f };
        const Vec4f randomPointOnCircle = GenerateBokeh(sample) * mDOF.aperture;
        origin = Vec4f::MulAndAdd(randomPointOnCircle.SplatX(), right, origin);
        origin = Vec4f::MulAndAdd(randomPointOnCircle.SplatY(), up, origin);

        direction = focusPoint - origin;
    }

    return Ray(origin, direction);
}

bool Camera::WorldToFilm(const Vec4f& worldPosition, Vec4f& outFilmCoords) const
{
    // TODO motion blur
    const Vec4f cameraSpacePosition = mWorldToScreen.TransformPoint(worldPosition);

    if (cameraSpacePosition.z > 0.0f)
    {
        // perspective projection
        outFilmCoords = BipolarToUnipolar(cameraSpacePosition / cameraSpacePosition.w);
        return true;
    }

    return false;
}

float Camera::PdfW(const Math::Vec4f& direction) const
{
    const float cosAtCamera = Vec4f::Dot3(GetLocalToWorld()[2], direction);

    // equivalent of:
    //const float imagePointToCameraDist = 0.5f / (mTanHalfFoV * cosAtCamera);
    //const float pdf = Sqr(imagePointToCameraDist) / (cosAtCamera * mAspectRatio);

    const float pdf = 0.25f / (Sqr(mTanHalfFoV) * Cube(cosAtCamera) * mAspectRatio);

    return Max(0.0f, pdf);
}


const Ray_Simd8 Camera::GenerateRay_Simd8(const Vec2x8f& coords, RenderingContext& context) const
{
    const Matrix4 transform = SampleTransform(context.time);

    Vec3x8f origin(transform.GetTranslation());
    Vec2x8f offsetedCoords = coords * 2.0f - Vec2x8f::One();

    // barrel distortion
    if (enableBarellDistortion)
    {
        Vec8f radius = Vec2x8f::Dot(offsetedCoords, offsetedCoords);
        radius *= (barrelDistortionConstFactor + barrelDistortionVariableFactor * context.randomGenerator.GetFloat());
        offsetedCoords += offsetedCoords * radius;
    }

    const Vec3x8f screenSpaceRayDir =
    {
        offsetedCoords.x * (mTanHalfFoV * mAspectRatio),
        offsetedCoords.y * mTanHalfFoV,
        Vec8f(1.0f)
    };

    // calculate ray direction (ideal, without DoF)
    Vec3x8f direction = transform.TransformVector(screenSpaceRayDir);

    // depth of field
    if (mDOF.enable)
    {
        const Vec3x8f focusPoint = Vec3x8f::MulAndAdd(direction, Vec8f(mDOF.focalPlaneDistance), origin);

        const Vec4f right = transform[0];
        const Vec4f up = transform[1];

        // TODO different bokeh shapes, texture, etc.
        const Vec2x8f randomPointOnCircle = GenerateBokeh_Simd8(context) * mDOF.aperture;
        origin = Vec3x8f::MulAndAdd(Vec3x8f(randomPointOnCircle.x), Vec3x8f(right), origin);
        origin = Vec3x8f::MulAndAdd(Vec3x8f(randomPointOnCircle.y), Vec3x8f(up), origin);

        direction = focusPoint - origin;
    }

    return Ray_Simd8(origin, direction);
}

//////////////////////////////////////////////////////////////////////////

const Vec4f Camera::GenerateBokeh(const Math::Vec3f sample) const
{
    switch (mDOF.bokehShape)
    {
        case BokehShape::Circle:
            return SamplingHelpers::GetCircle(sample);
        case BokehShape::Hexagon:
            return SamplingHelpers::GetHexagon(sample);
        case BokehShape::Square:
            return Vec4f::MulAndSub(Vec4f(sample), 2.0f, VECTOR_ONE);
        // TODO
        //case BokehShape::NGon:
        //    return SamplingHelpers::GetRegularPolygon(mDOF.apertureBlades, u);
        case BokehShape::Texture:
        {
            Vec4f coords;
            mDOF.bokehTexture->Sample(sample, coords);
            return UnipolarToBipolar(coords);
        }
    }

    NFE_FATAL("Invalid bokeh type");
    return Vec4f::Zero();
}

const Vec2x8f Camera::GenerateBokeh_Simd8(RenderingContext& context) const
{
    const Vec2x8f u(context.randomGenerator.GetVec8f(), context.randomGenerator.GetVec8f());

    switch (mDOF.bokehShape)
    {
        case BokehShape::Circle:
            return SamplingHelpers::GetCircle_Simd8(u);
        //case BokehShape::Hexagon:
        //    return SamplingHelpers::GetHexagon_Simd8(u, context.randomGenerator.GetVector8());
        case BokehShape::Square:
            return 2.0f * u - Vec2x8f(1.0f);
        //TODO
        //case BokehShape::NGon:
        //    return context.randomGenerator.GetRegularPolygon_Simd8(mDOF.apertureBlades);
    }

    NFE_FATAL("Invalid bokeh type");
    return Vec2x8f::Zero();
}

} // namespace RT
} // namespace NFE

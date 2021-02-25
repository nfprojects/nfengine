#include "PCH.h"
#include "Medium.h"
#include "PhaseFunction.h"
#include "../Rendering/RenderingContext.h"
#include "../Textures/Texture.h"
#include "../Textures/ConstTexture.h"
#include "../Common/Math/Transcendental.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"
#include "../Common/Reflection/Types/ReflectionUniquePtrType.hpp"
#include "../Common/Reflection/Types/ReflectionSharedPtrType.hpp"



NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::IMedium)
{
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::HomogenousEmissiveMedium)
{
    NFE_CLASS_PARENT(NFE::RT::IMedium);
    NFE_CLASS_MEMBER(mEmissionCoeff);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::HeterogeneousEmissiveMedium)
{
    NFE_CLASS_PARENT(NFE::RT::HomogenousEmissiveMedium);
    NFE_CLASS_MEMBER(mDensityTexture);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::HomogenousAbsorptiveMedium)
{
    NFE_CLASS_PARENT(NFE::RT::IMedium);
    NFE_CLASS_MEMBER(mExctinctionCoeff);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::HeterogeneousAbsorptiveMedium)
{
    NFE_CLASS_PARENT(NFE::RT::IMedium);
    NFE_CLASS_MEMBER(mDensityTexture);
    NFE_CLASS_MEMBER(mExctinctionCoeff);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::HomogenousScatteringMedium)
{
    NFE_CLASS_PARENT(NFE::RT::HomogenousAbsorptiveMedium);
    NFE_CLASS_MEMBER(mScatteringAlbedo);
    NFE_CLASS_MEMBER(mPhaseFunction);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::HeterogeneousScatteringMedium)
{
    NFE_CLASS_PARENT(NFE::RT::HeterogeneousAbsorptiveMedium);
    NFE_CLASS_MEMBER(mScatteringAlbedo);
    NFE_CLASS_MEMBER(mPhaseFunction);
}
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

////////////////////////////////////////////////////////////////////////////////////////

IMedium::~IMedium() = default;

////////////////////////////////////////////////////////////////////////////////////////

HomogenousEmissiveMedium::HomogenousEmissiveMedium(const HdrColorRGB emissionCoeff)
    : mEmissionCoeff(emissionCoeff)
{
    NFE_ASSERT(mEmissionCoeff.IsValid(), "");
}

const RayColor HomogenousEmissiveMedium::Sample(const Ray& ray, float minDistance, float maxDistance, MediumScatteringEvent& outScatteringEvent, RenderingContext& ctx) const
{
    NFE_UNUSED(ctx);

    const float totalDistance = maxDistance - minDistance;

    outScatteringEvent.direction = ray.dir;
    outScatteringEvent.distance = FLT_MAX;
    outScatteringEvent.transmittance = RayColor::One();

    if (mEmissionCoeff.IsBlack() || totalDistance >= FLT_MAX)
    {
        // avoid 0.0*(-inf) that leads to NAN
        outScatteringEvent.radiance = RayColor::Zero();
    }
    else
    {
        outScatteringEvent.radiance = totalDistance * RayColor::ResolveRGB(ctx.wavelength, mEmissionCoeff);
    }

    return RayColor::One();
}

const RayColor HomogenousEmissiveMedium::Transmittance(const Vec4f&, const Vec4f&, RenderingContext&) const
{
    return RayColor::One();
}

////////////////////////////////////////////////////////////////////////////////////////

HeterogeneousEmissiveMedium::HeterogeneousEmissiveMedium(const TexturePtr& densityTexture, const HdrColorRGB emissionCoeff)
    : HomogenousEmissiveMedium(emissionCoeff)
    , mDensityTexture(densityTexture)
{
}

const RayColor HeterogeneousEmissiveMedium::Sample(const Ray& ray, float minDistance, float maxDistance, MediumScatteringEvent& outScatteringEvent, RenderingContext& ctx) const
{
    const float totalDistance = maxDistance - minDistance;

    outScatteringEvent.direction = ray.dir;
    outScatteringEvent.distance = FLT_MAX;
    outScatteringEvent.transmittance = RayColor::One();

    if (mEmissionCoeff.IsBlack() || totalDistance >= FLT_MAX)
    {
        // avoid 0.0*(-inf) that leads to NAN
        outScatteringEvent.radiance = RayColor::Zero();
    }
    else
    {
        // sample random density
        const float u = ctx.randomGenerator.GetFloat();
        const float t = Lerp(minDistance, maxDistance, u);
        const Vec4f p = ray.GetAtDistance(t);
        const Vec4f density = mDensityTexture->Evaluate(p);

        outScatteringEvent.radiance = totalDistance * RayColor::ResolveRGB(ctx.wavelength, mEmissionCoeff.ToVec4f() * density);
    }

    return RayColor::One();
}

////////////////////////////////////////////////////////////////////////////////////////

HomogenousAbsorptiveMedium::HomogenousAbsorptiveMedium(const HdrColorRGB exctinctionCoeff)
    : mExctinctionCoeff(exctinctionCoeff)
{
    NFE_ASSERT(exctinctionCoeff.IsValid(), "");
}

const RayColor HomogenousAbsorptiveMedium::Sample(const Ray& ray, float minDistance, float maxDistance, MediumScatteringEvent& outScatteringEvent, RenderingContext& ctx) const
{
    outScatteringEvent.direction = ray.dir;
    outScatteringEvent.distance = FLT_MAX;
    outScatteringEvent.radiance = RayColor::Zero();
    outScatteringEvent.transmittance = Transmittance(maxDistance - minDistance, ctx);
    return outScatteringEvent.transmittance;
}

const RayColor HomogenousAbsorptiveMedium::Transmittance(const float distance, RenderingContext& ctx) const
{
    // TODO spectral rendering

    Vec4f coefficent = Vec4f::Zero();
    if (distance < FLT_MAX) // avoid 0.0*(-inf) that leads to NAN
    {
        coefficent = FastExp(-mExctinctionCoeff.ToVec4f() * distance);
    }

    return RayColor::ResolveRGB(ctx.wavelength, coefficent);
}

const RayColor HomogenousAbsorptiveMedium::Transmittance(const Vec4f& startPoint, const Vec4f& endPoint, RenderingContext& ctx) const
{
    const float distance = (startPoint - endPoint).Length3();

    return Transmittance(distance, ctx);
}

////////////////////////////////////////////////////////////////////////////////////////

HeterogeneousAbsorptiveMedium::HeterogeneousAbsorptiveMedium(const TexturePtr& densityTexture, const HdrColorRGB exctinctionCoeff)
    : mDensityTexture(densityTexture)
    , mExctinctionCoeff(exctinctionCoeff)
{
    NFE_ASSERT(exctinctionCoeff.IsValid(), "");

    if (!densityTexture)
    {
        mDensityTexture = MakeUniquePtr<ConstTexture>();
    }

    // TODO include max value in texture
    mInvMaxExctinction = 1.0f / Max(mExctinctionCoeff.r, mExctinctionCoeff.g, mExctinctionCoeff.b);
}

const RayColor HeterogeneousAbsorptiveMedium::Sample(const Ray& ray, float minDistance, float maxDistance, MediumScatteringEvent& outScatteringEvent, RenderingContext& ctx) const
{
    if (maxDistance > FLT_MAX)
    {
        return RayColor::Zero();
    }

    outScatteringEvent.direction = ray.dir;
    outScatteringEvent.distance = FLT_MAX;
    outScatteringEvent.radiance = RayColor::Zero();
    outScatteringEvent.transmittance = Transmittance(ray.GetAtDistance(minDistance), ray.GetAtDistance(maxDistance), ctx);

    NFE_ASSERT(outScatteringEvent.transmittance.IsValid(), "");

    return outScatteringEvent.transmittance;
}

const RayColor HeterogeneousAbsorptiveMedium::Transmittance(const Vec4f& startPoint, const Vec4f& endPoint, RenderingContext& ctx) const
{
    Vec4f transmittance(1.0f);

    Vec4f dir = endPoint - startPoint;
    const float distance = dir.Length3();

    if (distance > FLT_EPSILON)
    {
        dir /= distance;
        float t = 0.0f;

        const Vec4f scaledExctinctionCoeff = mExctinctionCoeff.ToVec4f() * mInvMaxExctinction;

        // ratio tracking
        for (;;)
        {
            const float u = ctx.randomGenerator.GetFloat();
            t -= Log(1.0f - u) * mInvMaxExctinction;
            if (t >= distance)
            {
                break;
            }

            const Vec4f p = startPoint + dir * t;
            const Vec4f density = mDensityTexture->Evaluate(p);
            transmittance *= Vec4f(1.0f) - scaledExctinctionCoeff * density;
        }
    }

    NFE_ASSERT(transmittance.IsValid(), "");
    NFE_ASSERT((transmittance >= Vec4f::Zero()).All(), "");
    NFE_ASSERT((transmittance <= Vec4f(1.0f)).All(), "");

    return RayColor::ResolveRGB(ctx.wavelength, transmittance);
}

////////////////////////////////////////////////////////////////////////////////////////

HomogenousScatteringMedium::HomogenousScatteringMedium(const HdrColorRGB exctinctionCoeff, const HdrColorRGB scatteringAlbedo)
    : HomogenousAbsorptiveMedium(exctinctionCoeff)
    , mPhaseFunction(MakeUniquePtr<HenyeyGreensteinPhaseFunction>())
    , mScatteringAlbedo(scatteringAlbedo)
{
    NFE_ASSERT(mScatteringAlbedo.IsValid(), "");
    //NFE_ASSERT((mScatteringAlbedo >= Vec4f::Zero()).All(), "");
    //NFE_ASSERT((mScatteringAlbedo <= Vec4f(1.0f)).All(), "");
}

const RayColor HomogenousScatteringMedium::Sample(const Ray& ray, float minDistance, float maxDistance, MediumScatteringEvent& outScatteringEvent, RenderingContext& ctx) const
{
    NFE_ASSERT(mScatteringAlbedo.IsValid(), "");
    NFE_ASSERT(mPhaseFunction, "No phase function");

    const float totalDistance = maxDistance - minDistance;
    NFE_ASSERT(totalDistance >= 0.0f, "");

    // TODO spectral rendering
    // https://cs.dartmouth.edu/~wjarosz/publications/novak14residual.pdf#cite.Carter72
    // find unbounded scatter location
    const uint32 channel = ctx.randomGenerator.GetInt() % Wavelength::NumComponents;
    const float xi = ctx.randomGenerator.GetFloat();
    const float selectedExctinctionCoeff = mExctinctionCoeff.ToVec4f()[channel];
    const float scatterDistance = (selectedExctinctionCoeff > 0.0f)
        ? (-Log(1.0f - xi) / selectedExctinctionCoeff)
        : std::numeric_limits<float>::infinity();

    // TODO emissive medium
    outScatteringEvent.radiance = RayColor::Zero();

    // check if sampled scatter location occurs before ray hits solid geometry
    const bool sampledMedium = scatterDistance < totalDistance;
    const float t = sampledMedium ? scatterDistance : totalDistance;

    outScatteringEvent.transmittance = Transmittance(t, ctx);

    RayColor mediumWeight = outScatteringEvent.transmittance;
    RayColor density = outScatteringEvent.transmittance;

    if (sampledMedium)
    {
        mPhaseFunction->Sample(-ray.dir, outScatteringEvent.direction, ctx.randomGenerator.GetVec2f());
        outScatteringEvent.distance = minDistance + scatterDistance;

        density *= RayColor::ResolveRGB(ctx.wavelength, mExctinctionCoeff);
        const float pdf = density.Average();
        const RayColor scatteringCoeff = RayColor::ResolveRGB(ctx.wavelength, mExctinctionCoeff) * RayColor::ResolveRGB(ctx.wavelength, mScatteringAlbedo);
        mediumWeight *= scatteringCoeff / pdf;
    }
    else
    {
        outScatteringEvent.direction = ray.dir;
        outScatteringEvent.distance = FLT_MAX;

        const float pdf = Max(FLT_EPSILON, density.Average());
        mediumWeight *= 1.0f / pdf;
    }

    NFE_ASSERT(mediumWeight.IsValid(), "");

    return mediumWeight;
}

////////////////////////////////////////////////////////////////////////////////////////

HeterogeneousScatteringMedium::HeterogeneousScatteringMedium(const TexturePtr& densityTexture, const HdrColorRGB exctinctionCoeff, const HdrColorRGB scatteringAlbedo)
    : HeterogeneousAbsorptiveMedium(densityTexture, exctinctionCoeff)
    , mPhaseFunction(MakeUniquePtr<HenyeyGreensteinPhaseFunction>())
    , mScatteringAlbedo(scatteringAlbedo)
{
    //NFE_ASSERT(mScatteringAlbedo.IsValid(), "");
    //NFE_ASSERT((mScatteringAlbedo >= Vec4f::Zero()).All(), "");
    //NFE_ASSERT((mScatteringAlbedo <= Vec4f(1.0f)).All(), "");
}

const RayColor HeterogeneousScatteringMedium::Sample(const Ray& ray, float minDistance, float maxDistance, MediumScatteringEvent& outScatteringEvent, RenderingContext& ctx) const
{
    NFE_ASSERT(mScatteringAlbedo.IsValid(), "");
    NFE_ASSERT(mPhaseFunction, "No phase function");

    // TODO emissive medium
    outScatteringEvent.radiance = RayColor::Zero();

    if (maxDistance < FLT_MAX) // TODO
    {
        const float invMaxDensity = 1.0f;

        float t = minDistance;
        while (true)
        {
            const Vec4f u = ctx.randomGenerator.GetVec4f();

            t -= Log(1.0f - u.x) * invMaxDensity / mExctinctionCoeff.Luminance();
            if (t >= maxDistance)
            {
                break;
            }

            const Vec4f p = ray.GetAtDistance(t);
            const Vec4f density = mDensityTexture->Evaluate(p);
            if (density.x * invMaxDensity > u.y) // TODO non-monochromatic density
            {
                mPhaseFunction->Sample(-ray.dir, outScatteringEvent.direction, Vec2f(u.z, u.w));
                outScatteringEvent.distance = t;

                return RayColor::ResolveRGB(ctx.wavelength, mScatteringAlbedo);
            }
        }
    }

    outScatteringEvent.direction = ray.dir;
    outScatteringEvent.distance = FLT_MAX;

    return RayColor::One();
}


} // namespace RT
} // namespace NFE

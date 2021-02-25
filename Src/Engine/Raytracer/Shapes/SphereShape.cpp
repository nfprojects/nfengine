#include "PCH.h"
#include "SphereShape.h"
#include "Rendering/ShadingData.h"
#include "../Common/Math/Geometry.hpp"
#include "../Common/Math/SamplingHelpers.hpp"
#include "../Common/Math/Transcendental.hpp"
#include "../Common/Math/Packed.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::SphereShape)
{
    NFE_CLASS_PARENT(NFE::RT::IShape);
    NFE_CLASS_MEMBER(mRadius).Min(0.000001f);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

SphereShape::SphereShape(const float radius)
    : mRadius(radius)
    , mInvRadius(1.0f / radius)
    , mRadiusD(radius)
{
    NFE_ASSERT(mRadius > 0.0f, "");
}

bool SphereShape::OnPropertyChanged(const Common::StringView propertyName)
{
    if (propertyName == "mRadius")
    {
        NFE_ASSERT(mRadius > 0.0f, "");
        mRadiusD = mRadius;
        mInvRadius = 1.0f / mRadius;
        return true;
    }

    return IShape::OnPropertyChanged(propertyName);
}

const Box SphereShape::GetBoundingBox() const
{
    return Box(Vec4f::Zero(), mRadius);
}

float SphereShape::GetSurfaceArea() const
{
    return 4.0f * NFE_MATH_PI * Sqr(mRadius);
}

bool SphereShape::Intersect(const Ray& ray, RenderingContext& renderingCtx, ShapeIntersection& outResult) const
{
    NFE_UNUSED(renderingCtx);

    const double v = Vec4f::Dot3(ray.dir, -ray.origin);
    const double det = mRadiusD * mRadiusD - (double)ray.origin.SqrLength3() + v * v;

    if (det <= 0.0)
    {
        return false;
    }

    const double sqrtDet = sqrt(det);
    outResult.nearDist = (float)(v - sqrtDet);
    outResult.farDist = (float)(v + sqrtDet);

    if (outResult.farDist > outResult.nearDist)
    {
        // sphere has no border, so generate dummy IDs
        const float t = outResult.nearDist > 0.0f ? outResult.nearDist : outResult.farDist;
        const Vec4f intersectionPoint = ray.GetAtDistance(t) * mInvRadius;
        const PackedUnitVector3 packedPosition = PackedUnitVector3::FromVector(intersectionPoint);

        outResult.subObjectId =
            static_cast<uint32>(packedPosition.u & 0xF0u) |
            (static_cast<uint32>(packedPosition.v & 0xF0u) >> 8u);

        return true;
    }

    return false;
}

bool SphereShape::Intersect(const Vec4f& point) const
{
    return point.SqrLength3() <= mRadius * mRadius;
}

const Vec4f SphereShape::SampleVolume(const Vec3f& u) const
{
    const Vec4f point = SamplingHelpers::GetBall(u);

    return point * mRadius;
}

const Vec4f SphereShape::SampleSurface(const Vec3f& u, Vec4f* outNormal, float* outPdf) const
{
    if (outPdf)
    {
        *outPdf = 1.0f / GetSurfaceArea();
    }

    const Vec4f point = SamplingHelpers::GetSphere(u);

    if (outNormal)
    {
        *outNormal = point;
    }

    return point * mRadius;
}

bool SphereShape::SampleSurface(const Vec4f& ref, const Vec3f& u, ShapeSampleResult& result) const
{
    const Vec4f centerDir = -ref; // direction to light center
    const float centerDistSqr = centerDir.SqrLength3();
    const float centerDist = sqrtf(centerDistSqr);

    if (centerDistSqr < Sqr(mRadius))
    {
        // TODO illuminate inside?
        return false;
    }

    const float phi = NFE_MATH_2PI * u.y;
    const Vec4f sinCosPhi = SinCos(phi);

    float sinThetaMaxSqr = Sqr(mRadius) / centerDistSqr;
    float cosThetaMax = sqrtf(1.0f - Clamp(sinThetaMaxSqr, 0.0f, 1.0f));
    float cosTheta = Lerp(cosThetaMax, 1.0f, u.x);
    float sinThetaSqr = 1.0f - Sqr(cosTheta);
    float sinTheta = sqrtf(sinThetaSqr);

    // generate ray direction in the cone uniformly
    const Vec4f w = centerDir / centerDist;
    Vec4f tangent, bitangent;
    BuildOrthonormalBasis(w, tangent, bitangent);
    result.direction = (tangent * sinCosPhi.y + bitangent * sinCosPhi.x) * sinTheta + w * cosTheta;
    result.direction.Normalize3();

    // calculate distance to hit point
    result.distance = centerDist * cosTheta - sqrtf(Max(0.0f, Sqr(mRadius) - centerDistSqr * sinThetaSqr));

    result.cosAtSurface = cosTheta;

    if (cosThetaMax > 0.999999f)
    {
        result.pdf = FLT_MAX;
    }
    else
    {
        result.pdf = SphereCapPdf(cosThetaMax);
    }

    return true;
}

float SphereShape::Pdf(const Vec4f& ref, const Vec4f& point) const
{
    const Vec4f rayDir = (point - ref).Normalized3();

    const Vec4f centerDir = -ref; // direction to light center
    const float centerDistSqr = centerDir.SqrLength3();
    const Vec4f normal = point.Normalized3();
    const float cosAtLight = Max(0.0f, Vec4f::Dot3(-rayDir, normal));

    const float sinThetaMaxSqr = Clamp(Sqr(mRadius) / centerDistSqr, 0.0f, 1.0f);
    const float cosThetaMax = sqrtf(1.0f - sinThetaMaxSqr);
    const float pdfW = SphereCapPdf(cosThetaMax);

    // TODO may convert W->A unnecessary
    return pdfW * cosAtLight / (point - ref).SqrLength3();
}

/*
void SphereShape::Traverse_Packet(const PacketTraversalContext& context, const uint32 objectID, const uint32 numActiveGroups) const
{
    for (uint32 i = 0; i < numActiveGroups; ++i)
    {
        RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[i]];
        const Ray_Simd8& ray = rayGroup.rays[1];

        const Vec8f v = Vec3x8f::Dot(ray.dir, -ray.origin);
        const Vec8f det = Vec8f(mRadius * mRadius) - Vec3x8f::Dot(ray.origin, ray.origin) + v * v;

        const VectorBool8 detSign = det > Vec8f::Zero();
        if (detSign.None())
        {
            continue;
        }

        const Vec8f sqrtDet = Vec8f::Sqrt(det);
        const Vec8f nearDist = v - sqrtDet;
        const Vec8f farDist = v + sqrtDet;
        const Vec8f t = Vec8f::Select(nearDist, farDist, nearDist < Vec8f::Zero());

        const VectorBool8 distMask = detSign & (t > Vec8f::Zero()) & (t < rayGroup.maxDistances);

        const Vec8f uCoord = Vec8f::Zero(); // TODO
        const Vec8f vCoord = Vec8f::Zero(); // TODO

        context.StoreIntersection(rayGroup, t, uCoord, vCoord, distMask, objectID);
    }
}
*/

void SphereShape::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outData) const
{
    NFE_UNUSED(hitPoint);

    outData.texCoord = CartesianToSphericalCoordinates(-outData.frame.GetTranslation());
    outData.frame[2] = outData.frame.GetTranslation() * mInvRadius;

    // equivalent of: Vec4f::Cross3(outData.normal, VECTOR_Y);
    outData.frame[0] = (outData.frame[2].Swizzle<2,0,0,0>() & Vec4f::MakeMask<1,0,1,0>()).ChangeSign<1,0,0,0>();

    outData.frame[1] = -Vec4f::Cross3(outData.frame[0], outData.frame[2]);

    // TODO is that needed?
    outData.frame[0].FastNormalize3();
    outData.frame[1].FastNormalize3();
    outData.frame[2].FastNormalize3();
}


} // namespace RT
} // namespace NFE

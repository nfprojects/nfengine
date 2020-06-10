#include "PCH.h"
#include "RectShape.h"
#include "Rendering/ShadingData.h"
#include "Traversal/TraversalContext.h"
#include "../Common/Math/Geometry.hpp"
#include "../Common/Math/SamplingHelpers.hpp"
#include "../Common/Math/SphericalQuad.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::RectShape)
{
    NFE_CLASS_PARENT(NFE::RT::IShape);
    NFE_CLASS_MEMBER(mSize);
    NFE_CLASS_MEMBER(mTextureScale);
    NFE_CLASS_MEMBER(mEnableSolidAngleSampling);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

RectShape::RectShape(const Vec2f size, const Vec2f texScale)
    : mSize(size)
    , mTextureScale(texScale)
    , mEnableSolidAngleSampling(false)
{
    NFE_ASSERT(mSize.x > 0.0f, "");
    NFE_ASSERT(mSize.y > 0.0f, "");
}

const Box RectShape::GetBoundingBox() const
{
    return Box(Vec4f(-mSize.x, -mSize.y, 0.0f, 0.0f), Vec4f(mSize.x, mSize.y, 0.0f, 0.0f));
}

float RectShape::GetSurfaceArea() const
{
    return 4.0f * mSize.x * mSize.y;
}

bool RectShape::Intersect(const Math::Ray& ray, RenderingContext& renderingCtx, ShapeIntersection& outResult) const
{
    NFE_UNUSED(renderingCtx);

    const float t = -ray.origin.z * ray.invDir.z;

    if (t > FLT_EPSILON)
    {
        const Vec4f pos = ray.GetAtDistance(t);

        if ((Vec4f::Abs(pos) < Vec4f(mSize)).GetMask() == 0x3)
        {
            outResult.nearDist = t;
            outResult.farDist = t;
            return true;
        }
    }

    return false;
}

const Vec4f RectShape::Sample(const Vec3f& u, Math::Vec4f* outNormal, float* outPdf) const
{
    if (outPdf)
    {
        *outPdf = 1.0f / GetSurfaceArea();
    }

    if (outNormal)
    {
        *outNormal = VECTOR_Z;
    }

    return Vec4f(mSize) * (2.0f * Vec4f(Vec2f(u)) - VECTOR_ONE);
}

bool RectShape::Sample(const Math::Vec4f& ref, const Math::Vec3f& u, ShapeSampleResult& result) const
{
    if (mEnableSolidAngleSampling)
    {
        SphericalQuad squad;
        squad.Init(Vec4f(-mSize.x, -mSize.y), Vec4f(2.0f * mSize.x, 0.0f), Vec4f(0.0f, 2.0f * mSize.y), ref);
        const Vec4f lightPoint = squad.Sample(u.x, u.y, result.pdf);

        result.direction = lightPoint - ref;
        result.normal = VECTOR_Z;
        const float sqrDistance = result.direction.SqrLength3();

        if (sqrDistance > Sqr(FLT_EPSILON))
        {
            const float distance = sqrtf(sqrDistance);
            result.direction /= distance;

            const float cosNormalDir = -result.direction.z;
            if (cosNormalDir > FLT_EPSILON)
            {
                result.distance = distance;
                result.cosAtSurface = cosNormalDir;
                return true;
            }
        }
        return false;
    }

    return IShape::Sample(ref, u, result);
}

/*
void PlaneSceneObject::Traverse_Packet(const PacketTraversalContext& context, const uint32 objectID, const uint32 numActiveGroups) const
{
    for (uint32 i = 0; i < numActiveGroups; ++i)
    {
        RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[i]];

        const Vec8f t = -rayGroup.rays[1].origin.y * rayGroup.rays[1].invDir.y;
        VectorBool8 mask = (t > Vec8f::Zero()) & (t < rayGroup.maxDistances);

        if (mask.None())
        {
            continue;
        }

        const Vec8f x = Vec8f::MulAndAdd(rayGroup.rays[1].dir.x, t, rayGroup.rays[1].origin.x);
        const Vec8f z = Vec8f::MulAndAdd(rayGroup.rays[1].dir.z, t, rayGroup.rays[1].origin.z);
        mask = mask & (Vec8f::Abs(x) < Vec8f(mSize.x)) & (Vec8f::Abs(z) < Vec8f(mSize.y));

        const Vec8f u = Vec8f::Zero(); // TODO
        const Vec8f v = Vec8f::Zero(); // TODO

        context.StoreIntersection(rayGroup, t, u, v, mask, objectID);
    }
}
*/

void RectShape::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outData) const
{
    NFE_UNUSED(hitPoint);

    outData.texCoord = (outData.frame.GetTranslation() & Vec4f::MakeMask<1, 1, 0, 0>()) * Vec4f(mTextureScale);
    outData.frame[0] = VECTOR_X;
    outData.frame[1] = VECTOR_Y;
    outData.frame[2] = VECTOR_Z;
}


} // namespace RT
} // namespace NFE

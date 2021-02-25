#include "PCH.h"
#include "CylinderShape.h"
#include "Rendering/ShadingData.h"
#include "Traversal/TraversalContext.h"
#include "../../Common/Math/Geometry.hpp"
#include "../../Common/Math/SamplingHelpers.hpp"
#include "../../Common/Math/Transcendental.hpp"
#include "../../Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::CylinderShape)
{
    NFE_CLASS_PARENT(NFE::RT::IShape);
    NFE_CLASS_MEMBER(mRadius).Min(0.000001f);
    NFE_CLASS_MEMBER(mHeight).Min(0.000001f);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

CylinderShape::CylinderShape(const float radius, const float height)
    : mRadius(radius)
    , mInvRadius(1.0f / radius)
    , mHeight(height)
{
    NFE_ASSERT(mRadius > 0.0f, "Radius should be higher than 0");
}

bool CylinderShape::OnPropertyChanged(const Common::StringView propertyName)
{
    if (propertyName == "mRadius")
    {
        mRadius = Max(mRadius, 0.000001f); // TODO this should be ensured by the RTTI
        mInvRadius = 1.0f / mRadius;
        return true;
    }
    else if (propertyName == "mHeight")
    {
        mHeight = Max(mHeight, 0.000001f); // TODO this should be ensured by the RTTI
        return true;
    }

    return IShape::OnPropertyChanged(propertyName);
}

const Box CylinderShape::GetBoundingBox() const
{
    return Box(Vec4f(-mRadius, -mRadius, 0.0f), Vec4f(mRadius, mRadius, mHeight));
}

float CylinderShape::GetSurfaceArea() const
{
    return NFE_MATH_2PI * mRadius * (mRadius + mHeight);
}

bool CylinderShape::Intersect(const Ray& ray, RenderingContext& renderingCtx, ShapeIntersection& outResult) const
{
    NFE_UNUSED(renderingCtx);

    float k2 = 1.0f - ray.dir.z * ray.dir.z;
    float k1 = Vec4f::Dot3(ray.origin, ray.dir) - ray.origin.z * ray.dir.z;
    float k0 = Vec4f::Dot3(ray.origin, ray.origin) - ray.origin.z * ray.origin.z - mRadius * mRadius;

    float h = k1 * k1 - k2 * k0;
    if (h >= 0.0f)
    {
        h = sqrtf(h);
        float t = (-k1 - h) / k2;

        // side
        float z = ray.origin.z + t * ray.dir.z;
        if (z > 0.0f && z < mHeight)
        {
            outResult.farDist = t;
            outResult.nearDist = t;
            outResult.subObjectId = 0;
            return true;
            //return Vec4f(t, (ray.origin + t * ray.dir - ba * y) / ra);
        }

        // caps
        t = ((z < 0.0f ? 0.0f : mHeight) - ray.origin.z) * ray.invDir.z;
        if (Abs(k1 + k2 * t) < h)
        {
            outResult.farDist = t;
            outResult.nearDist = t;
            outResult.subObjectId = z > 0.0f ? 1 : 2;
            return true;
            //return vec4(t, ba * sign(y));
        }
    }

    return false;
}

bool CylinderShape::Intersect(const Vec4f& point) const
{
    return point.SqrLength3() <= mRadius * mRadius;
}

const Vec4f CylinderShape::SampleVolume(const Vec3f& u) const
{
    Vec4f point = SamplingHelpers::GetDisk(u) * mRadius;
    point.z = u.z * mHeight;
    return point;
}

const Vec4f CylinderShape::SampleSurface(const Vec3f& u, Vec4f* outNormal, float* outPdf) const
{
    const float totalSurfaceArea = GetSurfaceArea();

    if (outPdf)
    {
        *outPdf = 1.0f / totalSurfaceArea;
    }

    const float capSurfaceArea = NFE_MATH_PI * Sqr(mRadius);

    Vec4f point, normal;

    if (u.z * totalSurfaceArea < capSurfaceArea) // top cap
    {
        point = mRadius * SamplingHelpers::GetDisk(u);
        point.z = mHeight;
        normal = VECTOR_Z;
    }
    else if (u.z * totalSurfaceArea < 2.0f * capSurfaceArea) // bottom cap
    {
        point = mRadius * SamplingHelpers::GetDisk(u);
        normal = VECTOR_NEG_Z;
    }
    else // side
    {
        normal = SamplingHelpers::GetCircle(u.x);
        point = mRadius * normal;
        point.z = u.y * mHeight;
    }

    if (outNormal)
    {
        *outNormal = normal;
    }

    return point;
}

void CylinderShape::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outData) const
{
    NFE_UNUSED(hitPoint);

    // TODO texture mapping
    outData.texCoord = Vec4f::Zero();

    if (hitPoint.subObjectId == 0) // side
    {
        outData.frame[2] = (outData.frame.GetTranslation() & Vec4f::MakeMask<1,1,0,0>()) * mInvRadius;
        outData.frame[0] = (outData.frame[2].Swizzle<1,0,2,3>() & Vec4f::MakeMask<1,1,0,0>()).ChangeSign<1,0,0,0>();
        outData.frame[1] = VECTOR_Z;

        // TODO is that needed?
        outData.frame[0].FastNormalize3();
        outData.frame[2].FastNormalize3();
    }
    else if (hitPoint.subObjectId == 1) // top cap
    {
        outData.frame[0] = VECTOR_X;
        outData.frame[1] = VECTOR_Y;
        outData.frame[2] = VECTOR_Z;
    }
    else if (hitPoint.subObjectId == 2) // bottom cap
    {
        outData.frame[0] = VECTOR_X;
        outData.frame[1] = VECTOR_NEG_Y;
        outData.frame[2] = VECTOR_NEG_Z;
    }
    else
    {
        NFE_FATAL("Invalid sub object ID %u", hitPoint.subObjectId);
    }
}


} // namespace RT
} // namespace NFE

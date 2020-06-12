#include "PCH.h"
#include "BoxShape.h"
#include "Rendering/ShadingData.h"
#include "../Common/Math/Geometry.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::BoxShape)
{
    NFE_CLASS_PARENT(NFE::RT::IShape);
    NFE_CLASS_MEMBER(mSize).Min(0.000001f);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

namespace helper
{

const Matrix4 g_faceFrames[] =
{
    { { 0.0f, 0.0f,  1.0f, 0.0f}, { 0.0f, 1.0f,  0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f, 0.0f} },
    { { 0.0f, 0.0f, -1.0f, 0.0f}, { 0.0f, 1.0f,  0.0f, 0.0f}, {+1.0f,  0.0f,  0.0f, 0.0f}, { 0.0f,  0.0f,  1.0f, 0.0f} },
    { {+1.0f, 0.0f,  0.0f, 0.0f}, { 0.0f, 0.0f,  1.0f, 0.0f}, { 0.0f, -1.0f,  0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f, 0.0f} },
    { {+1.0f, 0.0f,  0.0f, 0.0f}, { 0.0f, 0.0f, -1.0f, 0.0f}, { 0.0f, +1.0f,  0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f, 0.0f} },
    { {-1.0f, 0.0f,  0.0f, 0.0f}, { 0.0f, 1.0f,  0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f, 0.0f}, { 1.0f,  0.0f,  0.0f, 0.0f} },
    { {+1.0f, 0.0f,  0.0f, 0.0f}, { 0.0f, 1.0f,  0.0f, 0.0f}, { 0.0f,  0.0f, +1.0f, 0.0f}, {-1.0f,  0.0f,  0.0f, 0.0f} },
};

NFE_FORCE_INLINE int32 ConvertXYZtoCubeUV(const Vec4f& p, Vec4f& outUV)
{
    const Vec4f abs = Vec4f::Abs(p);
    const int32 isXPositive = p.x > 0 ? 1 : 0;
    const int32 isYPositive = p.y > 0 ? 1 : 0;
    const int32 isZPositive = p.z > 0 ? 1 : 0;

    float maxAxis, uc, vc;
    int32 side;

    if (abs.x >= abs.y && abs.x >= abs.z)
    {
        if (isXPositive) // +X
        {
            uc = -p.z;
        }
        else // -X
        {
            uc = p.z;
        }

        side = isXPositive;
        maxAxis = abs.x;
        vc = p.y;
    }
    else if (abs.y >= abs.x && abs.y >= abs.z)
    {
        if (isYPositive) // +Y
        {
            vc = -p.z;
        }
        else // -Y
        {
            vc = p.z;
        }

        side = isYPositive + 2;
        maxAxis = abs.y;
        uc = p.x;
    }
    else
    {
        if (isZPositive) // +Z
        {
            uc = p.x;
        }
        else // -Z
        {
            uc = -p.x;
        }

        side = isZPositive + 4;
        maxAxis = abs.z;
        vc = p.y;
    }

    // Convert range from -1 to 1 to 0 to 1
    outUV = Vec4f(uc, vc, 0.0f, 0.0f) / (2.0f * maxAxis) + Vec4f(0.5f);

    return side;
}

} // helper

BoxShape::BoxShape(const Vec4f& size)
    : mSize(size)
{
    OnSizeChanged();
}

bool BoxShape::OnPropertyChanged(const Common::StringView propertyName)
{
    if (propertyName == "mSize")
    {
        OnSizeChanged();
        return true;
    }

    return IShape::OnPropertyChanged(propertyName);
}

void BoxShape::OnSizeChanged()
{
    NFE_ASSERT(mSize.x > 0.0f, "");
    NFE_ASSERT(mSize.y > 0.0f, "");
    NFE_ASSERT(mSize.z > 0.0f, "");

    mInvSize = VECTOR_ONE / mSize;

    mSize.w = 0.0f;
    mInvSize.w = 0.0f;

    mFaceCdf.x = mSize.y * mSize.z;
    mFaceCdf.y = mFaceCdf.x + mSize.z * mSize.x;
    mFaceCdf.z = mFaceCdf.y + mSize.x * mSize.y;
}

const Box BoxShape::GetBoundingBox() const
{
    return Box(-mSize, mSize);
}

float BoxShape::GetSurfaceArea() const
{
    return 8.0f * (mSize.x * (mSize.y + mSize.z) + mSize.y * mSize.z);
}

bool BoxShape::Intersect(const Math::Ray& ray, RenderingContext& renderingCtx, ShapeIntersection& outResult) const
{
    NFE_UNUSED(renderingCtx);

    const Box box(-mSize, mSize);

    outResult.subObjectId = 0;

    return Intersect_BoxRay_TwoSided(ray, box, outResult.nearDist, outResult.farDist);
}

bool BoxShape::Intersect(const Math::Vec4f& point) const
{
    const Box box(-mSize, mSize);
    return box.Intersects(point);
}

const Vec4f BoxShape::Sample(const Vec3f& u, Math::Vec4f* outNormal, float* outPdf) const
{
    float v = u.z;

    // select dimension for the normal vector (Z axis in local space)
    uint32 zAxis;
    {
        // TODO could be optimized by storing normalized CDF

        v *= mFaceCdf.z;
        if (v < mFaceCdf.x)
        {
            v /= mFaceCdf.x;
            zAxis = 0;
        }
        else if (v < mFaceCdf.y)
        {
            v = (v - mFaceCdf.x) / (mFaceCdf.y - mFaceCdf.x);
            zAxis = 1;
        }
        else
        {
            v = (v - mFaceCdf.y) / (mFaceCdf.z - mFaceCdf.y);
            zAxis = 2;
        }
    }

    // compute remaining axes
    const uint32 xAxis = (zAxis + 1) % 3u;
    const uint32 yAxis = (zAxis + 2) % 3u;

    // generate normal vector (2 possible directions for already chosen axis)
    Vec4f normal = Vec4f::Zero();
    normal[zAxis] = v < 0.5f ? -1.0f : 1.0f;

    // generate position by filling up remaining coordinates
    Vec4f pos = Vec4f::Zero();
    pos[xAxis] = (2.0f * u.x - 1.0f) * mSize[xAxis];
    pos[yAxis] = (2.0f * u.y - 1.0f) * mSize[yAxis];
    pos[zAxis] = normal[zAxis] * mSize[zAxis];

    if (outPdf)
    {
        *outPdf = 1.0f / GetSurfaceArea();
    }

    if (outNormal)
    {
        *outNormal = normal;
    }

    return pos;
}

void BoxShape::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outData) const
{
    using namespace helper;

    NFE_UNUSED(hitPoint);

    const int32 side = ConvertXYZtoCubeUV(outData.frame.GetTranslation() * mInvSize, outData.texCoord);
    outData.frame[0] = g_faceFrames[side][0];
    outData.frame[1] = g_faceFrames[side][1];
    outData.frame[2] = g_faceFrames[side][2];
}


} // namespace RT
} // namespace NFE

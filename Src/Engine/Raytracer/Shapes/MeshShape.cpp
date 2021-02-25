#include "PCH.h"

#include "MeshShape.h"
#include "BVH/BVHBuilder.h"

#include "Rendering/ShadingData.h"
#include "Traversal/Traversal_Single.h"
#include "Traversal/Traversal_Packet.h"

#include "../Common/Math/Distribution.hpp"
#include "../Common/Math/SamplingHelpers.hpp"
#include "../Common/Math/PackedLoadVec4f.hpp"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::MeshShape)
{
    NFE_CLASS_PARENT(NFE::RT::IShape);
    // TODO resource path
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

MeshShape::MeshShape()
{
}

MeshShape::~MeshShape()
{
}

const Box MeshShape::GetBoundingBox() const
{
    return mBoundingBox;
}

bool MeshShape::Initialize(const MeshDesc& desc)
{
    mBoundingBox = Box::Empty();

    const Vec3f* positions = desc.vertexBufferDesc.positions;
    const uint32* indexBuffer = desc.vertexBufferDesc.vertexIndexBuffer;

    DynArray<Box> boxes;
    boxes.Reserve(desc.vertexBufferDesc.numTriangles);
    for (uint32 i = 0; i < desc.vertexBufferDesc.numTriangles; ++i)
    {
        const Vec4f v0(positions[indexBuffer[3 * i + 0]]);
        const Vec4f v1(positions[indexBuffer[3 * i + 1]]);
        const Vec4f v2(positions[indexBuffer[3 * i + 2]]);

        Box triBox(v0, v1, v2);

        boxes.PushBack(triBox);

        mBoundingBox = Box(mBoundingBox, triBox);
    }

    BVHBuilder::Indices newTrianglesOrder;
    BVHBuilder bvhBuilder(mBVH);
    if (!bvhBuilder.Build(boxes.Data(), desc.vertexBufferDesc.numTriangles, BvhBuildingParams(), newTrianglesOrder))
    {
        return false;
    }

    // calculate & print stats
    {
        BVH::Stats stats;
        mBVH.CalculateStats(stats);
        NFE_LOG_INFO("BVH stats:");
        NFE_LOG_INFO("    - max depth: %u", stats.maxDepth);
        NFE_LOG_INFO("    - total surface area: %f", stats.totalNodesArea);
        NFE_LOG_INFO("    - total volume: %f", stats.totalNodesVolume);

        std::stringstream str;
        for (uint32 i = 0; i < stats.leavesCountHistogram.Size(); ++i)
        {
            if (i > 0)
                str << ", ";
            str << i << " (" << stats.leavesCountHistogram[i] << ")";
        }
        NFE_LOG_INFO("    - leaf nodes histogram: %s", str.str().c_str());
    }

    // reorder triangles
    {
        DynArray<uint32> newIndexBuffer(desc.vertexBufferDesc.numTriangles * 3);
        DynArray<uint32> newMaterialIndexBuffer(desc.vertexBufferDesc.numTriangles);
        for (uint32 i = 0; i < desc.vertexBufferDesc.numTriangles; ++i)
        {
            const uint32 newTriangleIndex = newTrianglesOrder[i];
            NFE_ASSERT(newTriangleIndex < desc.vertexBufferDesc.numTriangles, "");

            newIndexBuffer[3 * i] = indexBuffer[3 * newTriangleIndex];
            newIndexBuffer[3 * i + 1] = indexBuffer[3 * newTriangleIndex + 1];
            newIndexBuffer[3 * i + 2] = indexBuffer[3 * newTriangleIndex + 2];
            newMaterialIndexBuffer[i] = desc.vertexBufferDesc.materialIndexBuffer[newTriangleIndex];
        }

        VertexBufferDesc vertexBufferDesc = desc.vertexBufferDesc;
        vertexBufferDesc.vertexIndexBuffer = newIndexBuffer.Data();
        vertexBufferDesc.materialIndexBuffer = newMaterialIndexBuffer.Data();

        if (!mVertexBuffer.Initialize(vertexBufferDesc))
        {
            NFE_LOG_ERROR("Failed to initialize vertex buffer");
            return false;
        }
    }

    // TODO reorder indices

    NFE_LOG_INFO("MeshShape '%s' created successfully", !desc.path.Empty() ? desc.path.Str() : "unnamed");
    return true;
}

float MeshShape::GetSurfaceArea() const
{
    return mSurfaceArea;
}

bool MeshShape::MakeSamplable()
{
    if (mImportanceMap)
    {
        return true;
    }

    NFE_LOG_INFO("MeshShape: Generating importance map for mesh '%s'...", mPath.Str());

    DynArray<float> importancePdf;
    importancePdf.Resize(mVertexBuffer.GetNumTriangles());

    double totalArea = 0.0;
    for (uint32 i = 0; i < mVertexBuffer.GetNumTriangles(); ++i)
    {
        const ProcessedTriangle& tri = mVertexBuffer.GetTriangle(i);
        const float triArea = TriangleSurfaceArea(Vec4f(tri.edge1), Vec4f(tri.edge2));
        importancePdf[i] = triArea;
        totalArea += triArea;
    }

    mSurfaceArea = static_cast<float>(totalArea);
    mSurfaceAreaInv = 1.0f / mSurfaceArea;

    mImportanceMap = MakeUniquePtr<Distribution>();
    return mImportanceMap->Initialize(importancePdf.Data(), importancePdf.Size());
}

const Vec4f MeshShape::SampleSurface(const Vec3f& u, Vec4f* outNormal, float* outPdf) const
{
    NFE_ASSERT(mImportanceMap, "Mesh is not samplable");

    float pdf = 0.0f;
    const uint32 triangleIndex = mImportanceMap->SampleDiscrete(u.z, pdf);
    NFE_ASSERT(triangleIndex < mVertexBuffer.GetNumTriangles(), "");

    const ProcessedTriangle& tri = mVertexBuffer.GetTriangle(triangleIndex);
    const Vec4f uv = SamplingHelpers::GetTriangle(u);

    const Vec4f v0 = Vec4f_Load_Vec3f_Unsafe(tri.v0);
    const Vec4f edge1 = Vec4f_Load_Vec3f_Unsafe(tri.edge1);
    const Vec4f edge2 = Vec4f_Load_Vec3f_Unsafe(tri.edge2);
    const Vec4f pos = v0 + edge1 * uv.x + edge2 * uv.y;

    if (outNormal)
    {
        *outNormal = Vec4f::Cross3(edge1, edge2).Normalized3();
    }

    if (outPdf)
    {
        *outPdf = mSurfaceAreaInv;
    }

    return pos;
}

void MeshShape::Traverse(const SingleTraversalContext& context, const uint32 objectID) const
{
    GenericTraverse<MeshShape>(context, objectID, this);
}

bool MeshShape::Intersect(const Ray& ray, RenderingContext& renderingCtx, ShapeIntersection& outResult) const
{
    HitPoint hitPoint;
    hitPoint.Reset();

    const SingleTraversalContext context =
    {
        ray,
        hitPoint,
        renderingCtx,
    };

    GenericTraverse<MeshShape>(context, 0, this);

    if (hitPoint.distance != HitPoint::DefaultDistance)
    {
        outResult.nearDist = hitPoint.distance;
        outResult.farDist = hitPoint.distance;
        outResult.u = hitPoint.u;
        outResult.v = hitPoint.v;
        outResult.subObjectId = 0;
        return true;
    }

    return false;
}

void MeshShape::Traverse(const PacketTraversalContext& context, const uint32 objectID, const uint32 numActiveGroups) const
{
    GenericTraverse<MeshShape, 1>(context, objectID, this, numActiveGroups);
}

void MeshShape::Traverse_Leaf(const SingleTraversalContext& context, const uint32 objectID, const BVH::Node& node) const
{
    float distance, u, v;

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
    context.context.localCounters.numRayTriangleTests += node.numLeaves;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS

    const uint32 numLeaves = node.numLeaves;
    const uint32 childIndex = node.childIndex;

    for (uint32 i = 0; i < numLeaves; ++i)
    {
        const uint32 triangleIndex = childIndex + i;
        const ProcessedTriangle& tri = mVertexBuffer.GetTriangle(triangleIndex);
        HitPoint& hitPoint = context.hitPoint;

        // filter triangle (to avoid self-intersections)
        if (triangleIndex == hitPoint.subObjectId && objectID == hitPoint.objectId)
        {
            continue;
        }

        if (Intersect_TriangleRay(context.ray, Vec4f(&tri.v0.x), Vec4f(&tri.edge1.x), Vec4f(&tri.edge2.x), u, v, distance))
        {
            if (distance < hitPoint.distance)
            {
                hitPoint.distance = distance;
                hitPoint.subObjectId = triangleIndex;
                hitPoint.objectId = objectID;
                hitPoint.u = u;
                hitPoint.v = v;

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
                context.context.localCounters.numPassedRayTriangleTests++;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS
            }
        }
    }
}

bool MeshShape::Traverse_Shadow(const SingleTraversalContext& context, const uint32 objectID) const
{
    return GenericTraverse_Shadow<MeshShape>(context, objectID, this);
}

bool MeshShape::Traverse_Leaf_Shadow(const SingleTraversalContext& context, const uint32 objectID, const BVH::Node& node) const
{
    float distance, u, v;

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
    context.context.localCounters.numRayTriangleTests += node.numLeaves;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS

    const uint32 numLeaves = node.numLeaves;
    const uint32 childIndex = node.childIndex;

    for (uint32 i = 0; i < numLeaves; ++i)
    {
        const uint32 triangleIndex = childIndex + i;
        const ProcessedTriangle& tri = mVertexBuffer.GetTriangle(triangleIndex);
        HitPoint& hitPoint = context.hitPoint;

        // filter triangle (to avoid self-intersections)
        if (triangleIndex == hitPoint.subObjectId && objectID == hitPoint.objectId)
        {
            continue;
        }

        if (Intersect_TriangleRay(context.ray, Vec4f(&tri.v0.x), Vec4f(&tri.edge1.x), Vec4f(&tri.edge2.x), u, v, distance))
        {
            if (distance < hitPoint.distance)
            {
                hitPoint.distance = distance;

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
                context.context.localCounters.numPassedRayTriangleTests++;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS

                return true;
            }
        }
    }

    return false;
}

/*
void MeshShape::Traverse_Leaf_Simd8(const SimdTraversalContext& context, const uint32 objectID, const BVH::Node& node) const
{
    const Vec8i objectIndexVec(objectID);

    Vec8f distance, u, v;
    Triangle_Simd8 tri;

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
    context.context.localCounters.numRayTriangleTests += 8 * node.numLeaves;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS

    for (uint32 i = 0; i < node.numLeaves; ++i)
    {
        SimdHitPoint& hitPoint = context.hitPoint;
        const uint32 triangleIndex = node.childIndex + i;
        const Vec8i triangleIndexVec(triangleIndex);

        mVertexBuffer.GetTriangle(triangleIndex, tri);

        const Vec8f mask = Intersect_TriangleRay_Simd8(context.ray.dir, context.ray.origin, tri, hitPoint.distance, u, v, distance);
        const uint32 intMask = mask.GetSignMask();

        // TODO triangle & object filtering
        if (intMask)
        {
            // combine results according to mask
            hitPoint.u = Vec8f::SelectBySign(hitPoint.u, u, mask);
            hitPoint.v = Vec8f::SelectBySign(hitPoint.v, v, mask);
            hitPoint.distance = Vec8f::SelectBySign(hitPoint.distance, distance, mask);
            hitPoint.subObjectId = Vec8i::SelectBySign(hitPoint.subObjectId, triangleIndexVec, Vec8i::Cast(mask));
            hitPoint.objectId = Vec8i::SelectBySign(hitPoint.objectId, objectIndexVec, Vec8i::Cast(mask));

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
            context.context.localCounters.numPassedRayTriangleTests += PopCount(intMask);
#endif // NFE_ENABLE_INTERSECTION_COUNTERS
        }
    }
}
*/

void MeshShape::Traverse_Leaf(const PacketTraversalContext& context, const uint32 objectID, const BVH::Node& node, const uint32 numActiveGroups) const
{
    RayPacketTypes::Float distance, u, v;
    RayPacketTypes::Triangle tri;

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
    context.context.localCounters.numRayTriangleTests += RayPacketTypes::GroupSize * node.numLeaves * numActiveGroups;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS

    for (uint32 i = 0; i < node.numLeaves; ++i)
    {
        const uint32 triangleIndex = node.childIndex + i;

        mVertexBuffer.GetTriangle(triangleIndex, tri);

        for (uint32 j = 0; j < numActiveGroups; ++j)
        {
            RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[j]];

            const auto mask = Simd<RayPacketTypes::GroupSize>::Intersect_TriangleRay(rayGroup.rays[1].dir, rayGroup.rays[1].origin, tri, rayGroup.maxDistances, u, v, distance);

            context.StoreIntersection(rayGroup, distance, u, v, mask, objectID, triangleIndex);

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
            context.context.localCounters.numPassedRayTriangleTests += PopCount(mask.GetMask());
#endif // NFE_ENABLE_INTERSECTION_COUNTERS
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

NFE_FORCE_NOINLINE
void MeshShape::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outData) const
 {
    VertexIndices indices;
    mVertexBuffer.GetVertexIndices(hitPoint.subObjectId, indices);

    if (indices.materialIndex != UINT32_MAX)
    {
        outData.material = mVertexBuffer.GetMaterial(indices.materialIndex);
    }

    VertexShadingData vertexShadingData[3];
    mVertexBuffer.GetShadingData(indices, vertexShadingData[0], vertexShadingData[1], vertexShadingData[2]);

    const Vec4f coeff1 = Vec4f(hitPoint.u);
    const Vec4f coeff2 = Vec4f(hitPoint.v);
    const Vec4f coeff0 = Vec4f(VECTOR_ONE) - (coeff1 + coeff2);

    const Vec4f texCoord0 = Vec4f_Load_Vec2f_Unsafe(vertexShadingData[0].texCoord);
    const Vec4f texCoord1 = Vec4f_Load_Vec2f_Unsafe(vertexShadingData[1].texCoord);
    const Vec4f texCoord2 = Vec4f_Load_Vec2f_Unsafe(vertexShadingData[2].texCoord);
    Vec4f texCoord = coeff1 * texCoord1;
    texCoord = Vec4f::MulAndAdd(coeff2, texCoord2, texCoord);
    texCoord = Vec4f::MulAndAdd(coeff0, texCoord0, texCoord);
    texCoord = texCoord.Swizzle<0,1,0,1>();
    NFE_ASSERT(texCoord.IsValid(), "");
    outData.texCoord = texCoord;

    const Vec4f tangent0 = Vec4f_Load_Vec3f_Unsafe(vertexShadingData[0].tangent);
    const Vec4f tangent1 = Vec4f_Load_Vec3f_Unsafe(vertexShadingData[1].tangent);
    const Vec4f tangent2 = Vec4f_Load_Vec3f_Unsafe(vertexShadingData[2].tangent);
    Vec4f tangent = coeff1 * tangent1;
    tangent = Vec4f::MulAndAdd(coeff2, tangent2, tangent);
    tangent = Vec4f::MulAndAdd(coeff0, tangent0, tangent);
    tangent.FastNormalize3();
    tangent = tangent.Swizzle<0, 1, 2, 0>();
    NFE_ASSERT(tangent.IsValid(), "");
    outData.frame[0] = tangent;

    const Vec4f normal0 = Vec4f_Load_Vec3f_Unsafe(vertexShadingData[0].normal);
    const Vec4f normal1 = Vec4f_Load_Vec3f_Unsafe(vertexShadingData[1].normal);
    const Vec4f normal2 = Vec4f_Load_Vec3f_Unsafe(vertexShadingData[2].normal);
    Vec4f normal = coeff1 * normal1;
    normal = Vec4f::MulAndAdd(coeff2, normal2, normal);
    normal = Vec4f::MulAndAdd(coeff0, normal0, normal);
    normal.Normalize3();
    normal = normal.Swizzle<0, 1, 2, 0>();
    NFE_ASSERT(normal.IsValid(), "");
    outData.frame[2] = normal;
}

} // namespace RT
} // namespace NFE

#include "PCH.h"

#include "MeshShape.h"
#include "BVH/BVHBuilder.h"

#include "Rendering/RenderingContext.h"
#include "Rendering/ShadingData.h"
#include "Traversal/TraversalContext.h"
#include "Traversal/Traversal_Single.h"

#include "../nfCommon/Math/Geometry.hpp"
#include "../nfCommon/Math/Simd8Geometry.hpp"
#include "../nfCommon/Math/Distribution.hpp"
#include "../nfCommon/Math/SamplingHelpers.hpp"
#include "../nfCommon/Math/Vector4Load.hpp"
#include "../nfCommon/Logger/Logger.hpp"
#include "../nfCommon/Reflection/ReflectionClassDefine.hpp"

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

    const Float3* positions = desc.vertexBufferDesc.positions;
    const uint32* indexBuffer = desc.vertexBufferDesc.vertexIndexBuffer;

    DynArray<Box> boxes;
    boxes.Reserve(desc.vertexBufferDesc.numTriangles);
    for (uint32 i = 0; i < desc.vertexBufferDesc.numTriangles; ++i)
    {
        const Vector4 v0(positions[indexBuffer[3 * i + 0]]);
        const Vector4 v1(positions[indexBuffer[3 * i + 1]]);
        const Vector4 v2(positions[indexBuffer[3 * i + 2]]);

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
            NFE_ASSERT(newTriangleIndex < desc.vertexBufferDesc.numTriangles);

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
        const float triArea = TriangleSurfaceArea(Vector4(tri.edge1), Vector4(tri.edge2));
        importancePdf[i] = triArea;
        totalArea += triArea;
    }

    mSurfaceArea = static_cast<float>(totalArea);
    mSurfaceAreaInv = 1.0f / mSurfaceArea;

    mImportanceMap = MakeUniquePtr<Distribution>();
    return mImportanceMap->Initialize(importancePdf.Data(), importancePdf.Size());
}

const Vector4 MeshShape::Sample(const Float3& u, Vector4* outNormal, float* outPdf) const
{
    NFE_ASSERT(mImportanceMap, "Mesh is not samplable");

    float pdf = 0.0f;
    const uint32 triangleIndex = mImportanceMap->SampleDiscrete(u.z, pdf);
    NFE_ASSERT(triangleIndex < mVertexBuffer.GetNumTriangles());

    const ProcessedTriangle& tri = mVertexBuffer.GetTriangle(triangleIndex);
    const Vector4 uv = SamplingHelpers::GetTriangle(u);

    const Vector4 v0 = Vector4_Load_Float3_Unsafe(tri.v0);
    const Vector4 edge1 = Vector4_Load_Float3_Unsafe(tri.edge1);
    const Vector4 edge2 = Vector4_Load_Float3_Unsafe(tri.edge2);
    const Vector4 pos = v0 + edge1 * uv.x + edge2 * uv.y;

    if (outNormal)
    {
        *outNormal = Vector4::Cross3(edge1, edge2).Normalized3();
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

        if (Intersect_TriangleRay(context.ray, Vector4(&tri.v0.x), Vector4(&tri.edge1.x), Vector4(&tri.edge2.x), u, v, distance))
        {
            HitPoint& hitPoint = context.hitPoint;

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

bool MeshShape::Traverse_Shadow(const SingleTraversalContext& context) const
{
    return GenericTraverse_Shadow<MeshShape>(context, this);
}

bool MeshShape::Traverse_Leaf_Shadow(const SingleTraversalContext& context, const BVH::Node& node) const
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
        if (Intersect_TriangleRay(context.ray, Vector4(&tri.v0.x), Vector4(&tri.edge1.x), Vector4(&tri.edge2.x), u, v, distance))
        {
            HitPoint& hitPoint = context.hitPoint;
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
    const VectorInt8 objectIndexVec(objectID);

    Vector8 distance, u, v;
    Triangle_Simd8 tri;

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
    context.context.localCounters.numRayTriangleTests += 8 * node.numLeaves;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS

    for (uint32 i = 0; i < node.numLeaves; ++i)
    {
        HitPoint_Simd8& hitPoint = context.hitPoint;
        const uint32 triangleIndex = node.childIndex + i;
        const VectorInt8 triangleIndexVec(triangleIndex);

        mVertexBuffer.GetTriangle(triangleIndex, tri);

        const Vector8 mask = Intersect_TriangleRay_Simd8(context.ray.dir, context.ray.origin, tri, hitPoint.distance, u, v, distance);
        const uint32 intMask = mask.GetSignMask();

        // TODO triangle & object filtering
        if (intMask)
        {
            // combine results according to mask
            hitPoint.u = Vector8::SelectBySign(hitPoint.u, u, mask);
            hitPoint.v = Vector8::SelectBySign(hitPoint.v, v, mask);
            hitPoint.distance = Vector8::SelectBySign(hitPoint.distance, distance, mask);
            hitPoint.subObjectId = VectorInt8::SelectBySign(hitPoint.subObjectId, triangleIndexVec, VectorInt8::Cast(mask));
            hitPoint.objectId = VectorInt8::SelectBySign(hitPoint.objectId, objectIndexVec, VectorInt8::Cast(mask));

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
            context.context.localCounters.numPassedRayTriangleTests += PopCount(intMask);
#endif // NFE_ENABLE_INTERSECTION_COUNTERS
        }
    }
}
*/

void MeshShape::Traverse_Leaf(const PacketTraversalContext& context, const uint32 objectID, const BVH::Node& node, const uint32 numActiveGroups) const
{
    Vector8 distance, u, v;
    Triangle_Simd8 tri;

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
    context.context.localCounters.numRayTriangleTests += 8 * node.numLeaves * numActiveGroups;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS

    for (uint32 i = 0; i < node.numLeaves; ++i)
    {
        const uint32 triangleIndex = node.childIndex + i;
        const Vector8 triangleIndexVec(triangleIndex);

        mVertexBuffer.GetTriangle(triangleIndex, tri);

        for (uint32 j = 0; j < numActiveGroups; ++j)
        {
            RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[j]];

            const VectorBool8 mask = Intersect_TriangleRay_Simd8(rayGroup.rays[1].dir, rayGroup.rays[1].origin, tri, rayGroup.maxDistances, u, v, distance);

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

    const Vector4 coeff1 = Vector4(hitPoint.u);
    const Vector4 coeff2 = Vector4(hitPoint.v);
    const Vector4 coeff0 = Vector4(VECTOR_ONE) - (coeff1 + coeff2);

    const Vector4 texCoord0 = Vector4_Load_Float2_Unsafe(vertexShadingData[0].texCoord);
    const Vector4 texCoord1 = Vector4_Load_Float2_Unsafe(vertexShadingData[1].texCoord);
    const Vector4 texCoord2 = Vector4_Load_Float2_Unsafe(vertexShadingData[2].texCoord);
    Vector4 texCoord = coeff1 * texCoord1;
    texCoord = Vector4::MulAndAdd(coeff2, texCoord2, texCoord);
    texCoord = Vector4::MulAndAdd(coeff0, texCoord0, texCoord);
    texCoord = texCoord.Swizzle<0,1,0,1>();
    NFE_ASSERT(texCoord.IsValid());
    outData.texCoord = texCoord;

    const Vector4 tangent0 = Vector4_Load_Float3_Unsafe(vertexShadingData[0].tangent);
    const Vector4 tangent1 = Vector4_Load_Float3_Unsafe(vertexShadingData[1].tangent);
    const Vector4 tangent2 = Vector4_Load_Float3_Unsafe(vertexShadingData[2].tangent);
    Vector4 tangent = coeff1 * tangent1;
    tangent = Vector4::MulAndAdd(coeff2, tangent2, tangent);
    tangent = Vector4::MulAndAdd(coeff0, tangent0, tangent);
    tangent.FastNormalize3();
    tangent = tangent.Swizzle<0, 1, 2, 0>();
    NFE_ASSERT(tangent.IsValid());
    outData.frame[0] = tangent;

    const Vector4 normal0 = Vector4_Load_Float3_Unsafe(vertexShadingData[0].normal);
    const Vector4 normal1 = Vector4_Load_Float3_Unsafe(vertexShadingData[1].normal);
    const Vector4 normal2 = Vector4_Load_Float3_Unsafe(vertexShadingData[2].normal);
    Vector4 normal = coeff1 * normal1;
    normal = Vector4::MulAndAdd(coeff2, normal2, normal);
    normal = Vector4::MulAndAdd(coeff0, normal0, normal);
    normal.Normalize3();
    normal = normal.Swizzle<0, 1, 2, 0>();
    NFE_ASSERT(normal.IsValid());
    outData.frame[2] = normal;
}

} // namespace RT
} // namespace NFE

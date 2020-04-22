#pragma once

#include "Shape.h"
#include "Mesh/VertexBuffer.h"

#include "../Traversal/HitPoint.h"
#include "../BVH/BVH.h"

#include "../../Common/Math/Box.hpp"
#include "../../Common/Math/Ray.hpp"
#include "../../Common/Math/SimdRay.hpp"
#include "../../Common/Containers/String.hpp"


namespace NFE {
namespace RT {

struct MeshDesc
{
    VertexBufferDesc vertexBufferDesc;
    Common::String path;
};

class NFE_ALIGN(16) MeshShape : public IShape
{
    NFE_DECLARE_POLYMORPHIC_CLASS(MeshShape);

public:
    NFE_RAYTRACER_API MeshShape();
    NFE_RAYTRACER_API ~MeshShape();

    // Initialize the mesh
    NFE_RAYTRACER_API bool Initialize(const MeshDesc& desc);

    // IShape
    virtual const Math::Box GetBoundingBox() const override;
    virtual float GetSurfaceArea() const override;
    virtual void Traverse(const SingleTraversalContext& context, const uint32 objectID) const override;
    virtual void Traverse(const PacketTraversalContext & context, const uint32 objectID, const uint32 numActiveGroups) const override;
    virtual bool Traverse_Shadow(const SingleTraversalContext& context) const override;
    virtual bool Intersect(const Math::Ray& ray, RenderingContext& renderingCtx, ShapeIntersection& outResult) const override;
    virtual bool MakeSamplable() override;
    virtual const Math::Vec4f Sample(const Math::Vec3f& u, Math::Vec4f * outNormal, float* outPdf = nullptr) const override;
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    NFE_FORCE_INLINE const BVH& GetBVH() const { return mBVH; }

    // Intersect ray(s) with BVH leaf
    void Traverse_Leaf(const SingleTraversalContext& context, const uint32 objectID, const BVH::Node& node) const;
    void Traverse_Leaf(const PacketTraversalContext& context, const uint32 objectID, const BVH::Node& node, const uint32 numActiveGroups) const;

    // Intersect shadow ray(s) with BVH leaf
    // Returns true if any hit was found
    bool Traverse_Leaf_Shadow(const SingleTraversalContext& context, const BVH::Node& node) const;

private:

    // bounding box after scaling
    Math::Box mBoundingBox;

    // vertex data
    VertexBuffer mVertexBuffer;

    // bounding volume hierarchy for tracing acceleration
    BVH mBVH;

    // importance map for triangle sampling
    Common::UniquePtr<Math::Distribution> mImportanceMap;

    float mSurfaceArea = 0.0f;
    float mSurfaceAreaInv = 0.0f;

    Common::String mPath;
};

using MeshShapePtr = Common::SharedPtr<MeshShape>;

} // namespace RT
} // namespace NFE

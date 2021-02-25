#pragma once

#include "SceneObject.h"

#include "../../../Common/Containers/SharedPtr.hpp"

namespace NFE {
namespace RT {

using ShapePtr = Common::SharedPtr<IShape>;
using MaterialPtr = Common::SharedPtr<Material>;
using MediumPtr = Common::SharedPtr<IMedium>;

class ShapeSceneObject : public ITraceableSceneObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ShapeSceneObject)

public:
    NFE_RAYTRACER_API ShapeSceneObject(const ShapePtr& shape);

    const IShape* GetShape() const { return mShape.Get(); }
    const Material* GetMaterial() const { return mMaterial.Get(); }
    const IMedium* GetMedium() const { return mMedium.Get(); }

    NFE_RAYTRACER_API void BindMaterial(const MaterialPtr& material);
    NFE_RAYTRACER_API void BindMedium(const MediumPtr& medium);

private:
    virtual Math::Box GetBoundingBox() const override;

    virtual void Traverse(const SingleTraversalContext& context, const uint32 objectID) const override;
    virtual void Traverse(const PacketTraversalContext& context, const uint32 objectID, const uint32 numActiveGroups) const override;

    virtual bool Traverse_Shadow(const SingleTraversalContext& context, const uint32 objectID) const override;

    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    ShapePtr mShape;
    MaterialPtr mMaterial;
    MediumPtr mMedium;
};

using ShapeSceneObjectPtr = Common::UniquePtr<ShapeSceneObject>;

} // namespace RT
} // namespace NFE

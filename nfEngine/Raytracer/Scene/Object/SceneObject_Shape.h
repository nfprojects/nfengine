#pragma once

#include "SceneObject.h"

#include "../../../nfCommon/Containers/SharedPtr.hpp"

namespace NFE {
namespace RT {

using ShapePtr = Common::SharedPtr<IShape>;
using MaterialPtr = Common::SharedPtr<Material>;

class ShapeSceneObject : public ITraceableSceneObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ShapeSceneObject);

public:
    NFE_RAYTRACER_API ShapeSceneObject(const ShapePtr& shape);

    NFE_RAYTRACER_API void SetDefaultMaterial(const MaterialPtr& material);

private:
    virtual Math::Box GetBoundingBox() const override;

    virtual void Traverse(const SingleTraversalContext& context, const uint32 objectID) const override;
    virtual void Traverse(const PacketTraversalContext& context, const uint32 objectID, const uint32 numActiveGroups) const override;

    virtual bool Traverse_Shadow(const SingleTraversalContext& context) const override;

    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    ShapePtr mShape;

    MaterialPtr mDefaultMaterial;
};

using ShapeSceneObjectPtr = Common::UniquePtr<ShapeSceneObject>;

} // namespace RT
} // namespace NFE

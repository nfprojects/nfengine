#pragma once

#include "SceneObject.h"

namespace NFE {
namespace RT {

using LightPtr = Common::UniquePtr<ILight>;

class LightSceneObject : public ITraceableSceneObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(LightSceneObject)

public:
    NFE_RAYTRACER_API explicit LightSceneObject(LightPtr light);

    NFE_FORCE_INLINE const ILight& GetLight() const { return *mLight; }

private:
    virtual Math::Box GetBoundingBox() const override;

    virtual void Traverse(const SingleTraversalContext& context, const uint32 objectID) const override;
    virtual void Traverse(const PacketTraversalContext& context, const uint32 objectID, const uint32 numActiveGroups) const override;

    virtual bool Traverse_Shadow(const SingleTraversalContext& context, const uint32 objectID) const override;

    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const override;

    LightPtr mLight;
};

} // namespace RT
} // namespace NFE

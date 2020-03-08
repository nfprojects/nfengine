#pragma once

#include "../../Raytracer.h"
#include "../../Utils/Memory.h"
#include "../../Traversal/HitPoint.h"
#include "../../../Common/Math/Box.hpp"
#include "../../../Common/Math/Matrix4.hpp"
#include "../../../Common/Math/Transform.hpp"
#include "../../../Common/Containers/UniquePtr.hpp"
#include "../../../Common/Memory/Aligned.hpp"
#include "../../../Common/Reflection/ReflectionClassDeclare.hpp"
#include "../../../Common/Reflection/Object.hpp"

namespace NFE {
namespace RT {

using SceneObjectPtr = Common::UniquePtr<ISceneObject>;

// Object on a scene
class ISceneObject
    : public Common::Aligned<16>
    , public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ISceneObject);

public:

    NFE_RAYTRACER_API ISceneObject();
    NFE_RAYTRACER_API virtual ~ISceneObject();

    NFE_RAYTRACER_API void SetTransform(const Math::Transform& transform);
    NFE_RAYTRACER_API void SetTransform(const Math::Matrix4& matrix);

    // Get world-space bounding box
    virtual Math::Box GetBoundingBox() const = 0;

    // get transform at time=0
    NFE_FORCE_INLINE const Math::Matrix4& GetBaseTransform() const { return mBaseTransform; }
    NFE_FORCE_INLINE const Math::Matrix4& GetBaseInverseTransform() const { return mInverseTranform; }

    // get transform at given point in time
    const Math::Matrix4 GetTransform(const float t) const;
    const Math::Matrix4 GetInverseTransform(const float t) const;

private:

    virtual bool OnPropertyChanged(const Common::StringView propertyName) override;

    Math::Matrix4 mBaseTransform;   // local->world transform at time=0.0
    Math::Matrix4 mInverseTranform; // world->local transform at time=0.0

    Math::Transform mTransform;

    // TODO velocity
};

class ITraceableSceneObject : public ISceneObject
{
public:
    // traverse the object and return hit points
    virtual void Traverse(const SingleTraversalContext& context, const uint32 objectID) const = 0;
    virtual void Traverse(const PacketTraversalContext& context, const uint32 objectID, const uint32 numActiveGroups) const = 0;

    // check shadow ray occlusion
    virtual bool Traverse_Shadow(const SingleTraversalContext& context) const = 0;

    // Calculate input data for shading routine
    // NOTE: all calculations are performed in local space
    // NOTE: frame[3] (translation) will be already filled, because it can be always calculated from ray distance
    virtual void EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const = 0;
};

} // namespace RT
} // namespace NFE

#pragma once

#include "../../Raytracer.h"
#include "../../Utils/Memory.h"
#include "../../Traversal/HitPoint.h"
#include "../../../nfCommon/Math/Box.hpp"
#include "../../../nfCommon/Math/Matrix4.hpp"
#include "../../../nfCommon/Containers/UniquePtr.hpp"
#include "../../../nfCommon/Memory/Aligned.hpp"

namespace NFE {
namespace RT {

using SceneObjectPtr = Common::UniquePtr<ISceneObject>;

// Object on a scene
class ISceneObject : public Common::Aligned<16>
{
public:
    enum class Type : uint8
    {
        Shape,
        Light,
        Decal,
    };

    NFE_RAYTRACER_API ISceneObject();
    NFE_RAYTRACER_API virtual ~ISceneObject();

    virtual Type GetType() const = 0;

    NFE_RAYTRACER_API void SetTransform(const Math::Matrix4& matrix);

    // Get world-space bounding box
    virtual Math::Box GetBoundingBox() const = 0;

    // get transform at time=0
    NFE_FORCE_INLINE const Math::Matrix4& GetBaseTransform() const { return mTransform; }
    NFE_FORCE_INLINE const Math::Matrix4& GetBaseInverseTransform() const { return mInverseTranform; }

    // get transform at given point in time
    const Math::Matrix4 GetTransform(const float t) const;
    const Math::Matrix4 GetInverseTransform(const float t) const;

private:
    Math::Matrix4 mTransform; // local->world transform at time=0.0
    Math::Matrix4 mInverseTranform;

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

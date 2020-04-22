#pragma once

#include "../Raytracer.h"

#include "../Color/RayColor.h"
#include "../Traversal/HitPoint.h"
#include "../BVH/BVH.h"
#include "../../Common/Containers/DynArray.hpp"
#include "../../Common/Containers/UniquePtr.hpp"
#include "../../Common/Memory/Aligned.hpp"

namespace NFE {
namespace RT {

using SceneObjectPtr = Common::UniquePtr<ISceneObject>;

/**
 * Rendering scene.
 * Allows for placing objects (meshes, lights, etc.) and raytracing them.
 */
class NFE_ALIGN(16) Scene : public Common::Aligned<16>
{
public:
    NFE_RAYTRACER_API Scene();
    NFE_RAYTRACER_API ~Scene();
    NFE_RAYTRACER_API Scene(Scene&&);
    NFE_RAYTRACER_API Scene& operator = (Scene&&);

    //NFE_RAYTRACER_API void AddLight(LightPtr object);
    NFE_RAYTRACER_API void AddObject(SceneObjectPtr object);

    NFE_RAYTRACER_API bool BuildBVH();

    NFE_FORCE_INLINE const BVH& GetBVH() const { return mTraceableObjectsBVH; }
    NFE_FORCE_INLINE const ITraceableSceneObject* GetHitObject(uint32 id) const { return mTraceableObjects[id]; }
    NFE_FORCE_INLINE const Common::DynArray<const LightSceneObject*>& GetLights() const { return mLights; }
    NFE_FORCE_INLINE const Common::DynArray<const LightSceneObject*>& GetGlobalLights() const { return mGlobalLights; }

    // find medium intersection for a given ray
    const IMedium* GetMedium(RenderingContext& context, const Math::Ray& ray, float solidGeometryDistance, float& outMinDistance, float& outMaxDistance) const;

    const IMedium* GetMediumAtPoint(const RenderingContext& context, const Math::Vec4f& p) const;

    // traverse the scene, returns hit points
    NFE_RAYTRACER_API void Traverse(const SingleTraversalContext& context) const;
    NFE_RAYTRACER_API void Traverse(const PacketTraversalContext& context) const;

    // cast shadow ray
    bool Traverse_Shadow(const SingleTraversalContext& context) const;

    NFE_RAYTRACER_API void EvaluateIntersection(const Math::Ray& ray, const HitPoint& hitPoint, const float time, IntersectionData& outIntersectionData) const;

    void TraceRay_Simd8(const RayPacketTypes::Ray& ray, RenderingContext& context, RayColor* outColors) const;

    void Traverse_Leaf(const SingleTraversalContext& context, const uint32 objectID, const BVH::Node& node) const;
    void Traverse_Leaf(const PacketTraversalContext& context, const uint32 objectID, const BVH::Node& node, uint32 numActiveGroups) const;

    bool Traverse_Leaf_Shadow(const SingleTraversalContext& context, const BVH::Node& node) const;

    void EvaluateShadingData(ShadingData& shadingData, RenderingContext& context) const;

private:
    Scene(const Scene&) = delete;
    Scene& operator = (const Scene&) = delete;

    NFE_FORCE_NOINLINE void Traverse_Object(const SingleTraversalContext& context, const uint32 objectID) const;
    NFE_FORCE_NOINLINE bool Traverse_Object_Shadow(const SingleTraversalContext& context, const uint32 objectID) const;

    void EvaluateDecals(ShadingData& shadingData, RenderingContext& context) const;

    // keeps ownership
    Common::DynArray<SceneObjectPtr> mAllObjects;

    Common::DynArray<const LightSceneObject*> mLights;
    Common::DynArray<const LightSceneObject*> mGlobalLights;

    Common::DynArray<const ITraceableSceneObject*> mTraceableObjects;
    BVH mTraceableObjectsBVH;

    Common::DynArray<const ShapeSceneObject*> mMediumObjects;

    Common::DynArray<const DecalSceneObject*> mDecals;
    BVH mDecalsBVH;
};

} // namespace RT
} // namespace NFE

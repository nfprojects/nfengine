#include "PCH.h"
#include "Scene.h"
#include "Medium/Medium.h"
#include "Light/BackgroundLight.h"
#include "Object/SceneObject_Shape.h"
#include "Object/SceneObject_Light.h"
#include "Object/SceneObject_Decal.h"
#include "Object/SceneObject_Shape.h"
#include "Rendering/ShadingData.h"
#include "BVH/BVHBuilder.h"
#include "Material/Material.h"
#include "Utils/Profiler.h"
#include "Shapes/Shape.h"

#include "Traversal/Traversal_Single.h"
#include "Traversal/Traversal_Packet.h"
#include "../Common/Reflection/ReflectionUtils.hpp"
#include "../Common/Reflection/Types/ReflectionClassType.hpp"

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

Scene::Scene() = default;

Scene::~Scene() = default;

Scene::Scene(Scene&&) = default;

Scene& Scene::operator = (Scene&&) = default;

void Scene::AddObject(SceneObjectPtr object)
{
    if (const LightSceneObject* lightObject = RTTI::Cast<LightSceneObject>(object.Get()))
    {
        mLights.PushBack(lightObject);
    }

    mAllObjects.PushBack(std::move(object));
}

bool Scene::BuildBVH()
{
    // determine objects to be added to the BVH
    mTraceableObjects.Clear();
    mDecals.Clear();
    mLights.Clear();
    mGlobalLights.Clear();
    for (const auto& object : mAllObjects)
    {
        if (const LightSceneObject* lightObject = RTTI::Cast<LightSceneObject>(object.Get()))
        {
            mLights.PushBack(lightObject);

            const ILight& light = lightObject->GetLight();

            const bool isFinite = (light.GetFlags() & ILight::Flag_IsFinite) != 0;
            const bool isDelta = (light.GetFlags() & ILight::Flag_IsDelta) != 0;

            if (isFinite && !isDelta)
            {
                mTraceableObjects.PushBack(static_cast<const ITraceableSceneObject*>(object.Get()));
            }
            else if (!isFinite)
            {
                mGlobalLights.PushBack(lightObject);
            }
        }
        else if (const ShapeSceneObject* shapeObject = RTTI::Cast<ShapeSceneObject>(object.Get()))
        {
            mTraceableObjects.PushBack(shapeObject);

            if (shapeObject->GetMedium())
            {
                mMediumObjects.PushBack(shapeObject);
			}
        }
		else if (const DecalSceneObject* decalObject = RTTI::Cast<DecalSceneObject>(object.Get()))
		{
            mDecals.PushBack(decalObject);
        }
    }

    // build BVH for traceable objects
    {
        DynArray<Box> boxes;
        for (const ISceneObject* obj : mTraceableObjects)
        {
            boxes.PushBack(obj->GetBoundingBox());
        }

        BVHBuilder::Indices newOrder;
        BVHBuilder bvhBuilder(mTraceableObjectsBVH);
        if (!bvhBuilder.Build(boxes.Data(), mTraceableObjects.Size(), BvhBuildingParams(), newOrder))
        {
            return false;
        }

        DynArray<const ITraceableSceneObject*> newObjectsArray;
        newObjectsArray.Reserve(mTraceableObjects.Size());
        for (uint32 i = 0; i < mTraceableObjects.Size(); ++i)
        {
            uint32 sourceIndex = newOrder[i];
            newObjectsArray.PushBack(mTraceableObjects[sourceIndex]);
        }
        mTraceableObjects = std::move(newObjectsArray);
    }

    // build BVH for decals
    {
        DynArray<Box> boxes;
        for (const DecalSceneObject* decal : mDecals)
        {
            boxes.PushBack(decal->GetBoundingBox());
        }

        BvhBuildingParams params;
        params.heuristics = BvhBuildingParams::Heuristics::Volume;

        BVHBuilder::Indices newOrder;
        BVHBuilder bvhBuilder(mDecalsBVH);
        if (!bvhBuilder.Build(boxes.Data(), boxes.Size(), params, newOrder))
        {
            return false;
        }

        DynArray<const DecalSceneObject*> newObjectsArray;
        newObjectsArray.Reserve(mDecals.Size());
        for (uint32 i = 0; i < mDecals.Size(); ++i)
        {
            uint32 sourceIndex = newOrder[i];
            newObjectsArray.PushBack(mDecals[sourceIndex]);
        }
        mDecals = std::move(newObjectsArray);
    }

    return true;
}

void Scene::Traverse_Object(const SingleTraversalContext& context, const uint32 objectID) const
{
    const ITraceableSceneObject* object = mTraceableObjects[objectID];

    const Matrix4 invTransform = object->GetInverseTransform(context.context.time);

    // transform ray to local-space
    const Ray transformedRay = invTransform.TransformRay_Unsafe(context.ray);

    const SingleTraversalContext objectContext =
    {
        transformedRay,
        context.hitPoint,
        context.context
    };

    object->Traverse(objectContext, objectID);
}

bool Scene::Traverse_Object_Shadow(const SingleTraversalContext& context, const uint32 objectID) const
{
    const ITraceableSceneObject* object = mTraceableObjects[objectID];

    const Matrix4 invTransform = object->GetInverseTransform(context.context.time);

    // transform ray to local-space
    Ray transformedRay = invTransform.TransformRay_Unsafe(context.ray);
    transformedRay.originDivDir = transformedRay.origin * transformedRay.invDir;

    const SingleTraversalContext objectContext =
    {
        transformedRay,
        context.hitPoint,
        context.context
    };

    return object->Traverse_Shadow(objectContext);
}

void Scene::Traverse_Leaf(const SingleTraversalContext& context, const uint32 objectID, const BVH::Node& node) const
{
    NFE_UNUSED(objectID);

    const uint32 numLeaves = node.numLeaves;
    const uint32 firstChild = node.childIndex;

    for (uint32 i = 0; i < numLeaves; ++i)
    {
        Traverse_Object(context, firstChild + i);
    }
}

bool Scene::Traverse_Leaf_Shadow(const SingleTraversalContext& context, const BVH::Node& node) const
{
    const uint32 numLeaves = node.numLeaves;
    const uint32 firstChild = node.childIndex;

    for (uint32 i = 0; i < numLeaves; ++i)
    {
        if (Traverse_Object_Shadow(context, firstChild + i))
        {
            return true;
        }
    }

    return false;
}

void Scene::Traverse_Leaf(const PacketTraversalContext& context, const uint32 objectID, const BVH::Node& node, uint32 numActiveGroups) const
{
    NFE_UNUSED(objectID);

    for (uint32 i = 0; i < node.numLeaves; ++i)
    {
        const uint32 objectIndex = node.childIndex + i;
        const ITraceableSceneObject* object = mTraceableObjects[objectIndex];
        const Matrix4 invTransform = object->GetInverseTransform(context.context.time);

        // transform ray to local-space
        for (uint32 j = 0; j < numActiveGroups; ++j)
        {
            RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[j]];
            rayGroup.rays[1].origin = invTransform.TransformPoint(rayGroup.rays[0].origin);
            rayGroup.rays[1].dir = invTransform.TransformVector(rayGroup.rays[0].dir);
            rayGroup.rays[1].invDir = Vec3x8f::FastReciprocal(rayGroup.rays[1].dir);
        }

        object->Traverse(context, objectIndex, numActiveGroups);
    }
}

void Scene::Traverse(const SingleTraversalContext& context) const
{
    context.context.localCounters.Reset();

    const uint32 numObjects = mTraceableObjects.Size();

    if (numObjects == 0)
    {
        // scene is empty
    }
    else if (numObjects == 1)
    {
        // bypass BVH
        Traverse_Object(context, 0);
    }
    else
    {
        // full BVH traversal
        GenericTraverse(context, 0, this);
    }

    context.context.counters.Append(context.context.localCounters);
}

bool Scene::Traverse_Shadow(const SingleTraversalContext& context) const
{
    const uint32 numObjects = mTraceableObjects.Size();

    if (numObjects == 0) // scene is empty
    {
        return false;
    }
    else if (numObjects == 1) // bypass BVH
    {
        return Traverse_Object_Shadow(context, 0);
    }
    else // full BVH traversal
    {
        return GenericTraverse_Shadow(context, this);
    }
}

void Scene::Traverse(const PacketTraversalContext& context) const
{
    const uint32 numObjects = mTraceableObjects.Size();

    const uint32 numRayGroups = context.ray.GetNumGroups();
    for (uint32 i = 0; i < numRayGroups; ++i)
    {
        context.ray.groups[i].maxDistances = VECTOR8_MAX;
        context.context.activeGroupsIndices[i] = (uint16)i;
    }

    for (uint32 i = 0; i < context.ray.numRays; ++i)
    {
        context.context.hitPoints[i].distance = FLT_MAX;
        context.context.hitPoints[i].objectId = UINT32_MAX;
    }

    if (numObjects == 0) // scene is empty
    {
        return;
    }
    else if (numObjects == 1) // bypass BVH
    {
        const ISceneObject* object = mTraceableObjects.Front();
        const Matrix4 invTransform = object->GetInverseTransform(context.context.time);

        for (uint32 j = 0; j < numRayGroups; ++j)
        {
            RayGroup& rayGroup = context.ray.groups[context.context.activeGroupsIndices[j]];
            rayGroup.rays[1].origin = invTransform.TransformPoint(rayGroup.rays[0].origin);
            rayGroup.rays[1].dir = invTransform.TransformVector(rayGroup.rays[0].dir);
            rayGroup.rays[1].invDir = Vec3x8f::FastReciprocal(rayGroup.rays[1].dir);
        }

        mTraceableObjects.Front()->Traverse(context, 0, numRayGroups);
    }
    else // full BVH traversal
    {
        GenericTraverse<Scene, 0>(context, 0, this, numRayGroups);
    }
}

void Scene::EvaluateIntersection(const Ray& ray, const HitPoint& hitPoint, const float time, IntersectionData& outData) const
{
    //NFE_SCOPED_TIMER(Scene_EvaluateIntersection);

    NFE_ASSERT(hitPoint.distance >= 0.0f && hitPoint.distance < FLT_MAX);
    NFE_ASSERT(hitPoint.u >= 0.0f && hitPoint.u <= 1.0f);
    NFE_ASSERT(hitPoint.v >= 0.0f && hitPoint.v <= 1.0f);

    const ITraceableSceneObject* object = mTraceableObjects[hitPoint.objectId];

    const Matrix4 transform = object->GetTransform(time);
    const Matrix4 invTransform = transform.FastInverseNoScale();

    const Vec4f worldPosition = ray.GetAtDistance(hitPoint.distance);
    outData.frame[3] = invTransform.TransformPoint(worldPosition);

    // calculate normal, tangent, tex coord, etc. from intersection data
    object->EvaluateIntersection(hitPoint, outData);
    {
        NFE_ASSERT(outData.texCoord.IsValid());
        NFE_ASSERT(outData.frame[0].IsValid());
        NFE_ASSERT(outData.frame[2].IsValid());
        NFE_ASSERT(Abs(1.0f - outData.frame[0].SqrLength3()) < 0.001f);
        NFE_ASSERT(Abs(1.0f - outData.frame[2].SqrLength3()) < 0.001f);
    }

    Vec4f localSpaceTangent = outData.frame[0];
    Vec4f localSpaceNormal = outData.frame[2];
    Vec4f localSpaceBitangent = Vec4f::Cross3(localSpaceTangent, localSpaceNormal);

    // apply normal mapping
    if (outData.material && outData.material->normalMap)
    {
        const Vec4f localNormal = outData.material->GetNormalVector(outData.texCoord);

        // transform normal vector
        Vec4f newNormal = localSpaceTangent * localNormal.x;
        newNormal = Vec4f::MulAndAdd(localSpaceBitangent, localNormal.y, newNormal);
        newNormal = Vec4f::MulAndAdd(localSpaceNormal, localNormal.z, newNormal);
        localSpaceNormal = newNormal.FastNormalized3();
    }

    // orthogonalize tangent vector (required due to normal/tangent vectors interpolation and normal mapping)
    // TODO this can be skipped if the tangent vector is the same for every point on the triangle (flat shading)
    // and normal mapping is disabled
    localSpaceTangent = Vec4f::Orthogonalize(localSpaceTangent, localSpaceNormal).Normalized3();

    // transform shading data from local space to world space
    outData.frame[2] = transform.TransformVector(localSpaceNormal);
    outData.frame[0] = transform.TransformVector(localSpaceTangent);
    outData.frame[1] = Vec4f::Cross3(outData.frame[0], outData.frame[2]);
    outData.frame[3] = worldPosition;

    // make sure the frame is orthonormal
    {
        // validate length
        NFE_ASSERT(Abs(1.0f - outData.frame[0].SqrLength3()) < 0.001f);
        NFE_ASSERT(Abs(1.0f - outData.frame[1].SqrLength3()) < 0.001f);
        NFE_ASSERT(Abs(1.0f - outData.frame[2].SqrLength3()) < 0.001f);

        // validate perpendicularity
        NFE_ASSERT(Vec4f::Dot3(outData.frame[1], outData.frame[0]) < 0.001f);
        //NFE_ASSERT(Vec4f::Dot3(outData.frame[1], outData.frame[2]) < 0.001f);
        //NFE_ASSERT(Vec4f::Dot3(outData.frame[2], outData.frame[0]) < 0.001f);

        // validate headness
        //const Vec4f computedNormal = Vec4f::Cross3(outData.frame[0], outData.frame[1]);
        //NFE_ASSERT((computedNormal - outData.frame[2]).SqrLength3() < 0.001f);
    }
}

void Scene::EvaluateShadingData(ShadingData& shadingData, RenderingContext& context) const
{
    if (shadingData.intersection.material)
    {
        shadingData.intersection.material->EvaluateShadingData(context.wavelength, shadingData);

        EvaluateDecals(shadingData, context);
    }
}

void Scene::EvaluateDecals(ShadingData& shadingData, RenderingContext& context) const
{
    if (mDecals.Empty())
    {
        return; // no decals
    }

    const uint32 maxOverlappingDecals = 8;
    uint32 numOverlappingDecals = 0;
    const DecalSceneObject* decals[maxOverlappingDecals];

    // collect overlapping decals

    const Vec4f& point = shadingData.intersection.frame.GetTranslation();

    // all nodes
    const BVH::Node* __restrict nodes = mDecalsBVH.GetNodes();

    // "nodes to visit" stack
    uint32 stackSize = 0;
    const BVH::Node* __restrict nodesStack[BVH::MaxDepth];

    // BVH traversal
    for (const BVH::Node* __restrict currentNode = nodes;;)
    {
        if (currentNode->IsLeaf())
        {
            const uint32 numLeaves = currentNode->numLeaves;
            const uint32 firstChild = currentNode->childIndex;

            for (uint32 i = 0; i < numLeaves; ++i)
            {
                NFE_ASSERT(numOverlappingDecals < maxOverlappingDecals);
                decals[numOverlappingDecals++] = mDecals[firstChild + i];
            }
        }
        else
        {
            const BVH::Node* __restrict childA = nodes + currentNode->childIndex;
            const BVH::Node* __restrict childB = childA + 1;

            bool hitA = childA->GetBox().Intersects(point);
            bool hitB = childB->GetBox().Intersects(point);

            if (hitA && hitB)
            {
                currentNode = childA;
                nodesStack[stackSize++] = childB;
                continue;
            }
            if (hitA)
            {
                currentNode = childA;
                continue;
            }
            if (hitB)
            {
                currentNode = childB;
                continue;
            }
        }

        if (stackSize == 0)
        {
            break;
        }

        // pop a node
        currentNode = nodesStack[--stackSize];
    }

    if (numOverlappingDecals > 0u)
    {
        struct DecalSorter
        {
            NFE_FORCE_INLINE bool operator() (const DecalSceneObject* a, const DecalSceneObject* b) const
            {
                return a->order > b->order;
            }
        };
        std::sort(decals, decals + numOverlappingDecals, DecalSorter());

        // apply decals
        for (uint32 i = 0; i < numOverlappingDecals; ++i)
        {
            decals[i]->Apply(shadingData, context);
        }
    }
}

const IMedium* Scene::GetMediumAtPoint(const RenderingContext& context, const Math::Vec4f& p) const
{
    // TODO what in case of overlapping media?

    for (const ShapeSceneObject* object : mMediumObjects)
    {
        const IMedium* medium = object->GetMedium();
        NFE_ASSERT(medium);

        // transform point to local-space
        const Matrix4 invTransform = object->GetInverseTransform(context.time);
        const Vec4f localPoint = invTransform.TransformPoint(p);

        if (object->GetShape()->Intersect(p))
        {
            return medium;
        }
    }

    return nullptr;
}

const IMedium* Scene::GetMedium(RenderingContext& context, const Ray& ray, float solidGeometryDistance, float& outMinDistance, float& outMaxDistance) const
{
    // TODO proper BVH for volumes set
    // TODO handling overlapping volumes

    /*
    for (const MediumSceneObject* object : mMediumObjects)
    {
        // transform ray to local-space
        const Matrix4 invTransform = object->GetInverseTransform(context.time);
        const Ray transformedRay = invTransform.TransformRay_Unsafe(ray);

        ShapeIntersection shapeIntersection;
        if (object->GetShape()->Intersect(transformedRay, shapeIntersection))
        {
            if (shapeIntersection.farDist > 0.0f && shapeIntersection.nearDist < solidGeometryDistance)
            {
                // clamp to ray origin and solid geometry
                outMinDistance = Max(shapeIntersection.nearDist, 0.0f);
                outMaxDistance = Min(shapeIntersection.farDist, solidGeometryDistance);

                return object->GetMedium();
            }
        }
    }
    */

    (void)context;
    (void)ray;
    (void)solidGeometryDistance;
    (void)outMinDistance;
    (void)outMaxDistance;

    return nullptr;
}

} // namespace RT
} // namespace NFE

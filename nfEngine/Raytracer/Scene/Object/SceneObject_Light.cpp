#include "PCH.h"
#include "SceneObject_Light.h"
#include "../Light/Light.h"
#include "../Light/AreaLight.h"
#include "../../Shapes/Shape.h"
#include "Traversal/TraversalContext.h"

namespace NFE {
namespace RT {

using namespace Math;

LightSceneObject::LightSceneObject(LightPtr light)
    : mLight(std::move(light))
{ }

ISceneObject::Type LightSceneObject::GetType() const
{
    return Type::Light;
}

Box LightSceneObject::GetBoundingBox() const
{
    const Box localSpaceBox = mLight->GetBoundingBox();
    return { GetBaseTransform().TransformBox(localSpaceBox), GetTransform(1.0f).TransformBox(localSpaceBox) };
}

void LightSceneObject::Traverse(const SingleTraversalContext& context, const uint32 objectID) const
{
    float lightDistance;
    if (mLight->TestRayHit(context.ray, lightDistance))
    {
        if (lightDistance > 0.0f && lightDistance < context.hitPoint.distance)
        {
            // mark as light
            context.hitPoint.Set(lightDistance, objectID, NFE_LIGHT_OBJECT);
        }
    }
}

bool LightSceneObject::Traverse_Shadow(const SingleTraversalContext& context) const
{
    float lightDistance;
    if (mLight->TestRayHit(context.ray, lightDistance))
    {
        if (lightDistance < context.hitPoint.distance)
        {
            context.hitPoint.distance = lightDistance;
            return true;
        }
    }

    return false;
}

void LightSceneObject::Traverse(const PacketTraversalContext& context, const uint32 objectID, const uint32 numActiveGroups) const
{
    NFE_UNUSED(context);
    NFE_UNUSED(objectID);
    NFE_UNUSED(numActiveGroups);
    // TODO
}

void LightSceneObject::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const
{
    if (mLight->GetType() == ILight::Type::Area)
    {
        const AreaLight& areaLight = static_cast<const AreaLight&>(*mLight);
        areaLight.GetShape()->EvaluateIntersection(hitPoint, outIntersectionData);
    }
    else
    {
        NFE_FATAL("Cannot evaluate intersection for non-area lights");
    }
}


} // namespace RT
} // namespace NFE

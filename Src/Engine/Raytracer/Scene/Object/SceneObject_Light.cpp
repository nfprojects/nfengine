#include "PCH.h"
#include "SceneObject_Light.h"
#include "../Light/AreaLight.h"
#include "../../Shapes/Shape.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"
#include "../Common/Reflection/Types/ReflectionUniquePtrType.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::LightSceneObject)
{
    NFE_CLASS_PARENT(NFE::RT::ISceneObject);
    NFE_CLASS_MEMBER(mLight).NonNull();
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

LightSceneObject::LightSceneObject(LightPtr light)
    : mLight(std::move(light))
{ }

Box LightSceneObject::GetBoundingBox() const
{
    const Box localSpaceBox = mLight->GetBoundingBox();
    return { GetBaseTransform().TransformBox(localSpaceBox), GetTransform(1.0f).TransformBox(localSpaceBox) };
}

void LightSceneObject::Traverse(const SingleTraversalContext& context, const uint32 objectID) const
{
    mLight->Traverse(context, objectID);
}

bool LightSceneObject::Traverse_Shadow(const SingleTraversalContext& context, const uint32 objectID) const
{
    NFE_UNUSED(objectID);

    return mLight->Traverse_Shadow(context, objectID);
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
    if (mLight->GetDynamicType() == RTTI::GetType<AreaLight>())
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

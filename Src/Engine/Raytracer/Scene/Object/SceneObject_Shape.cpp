#include "PCH.h"
#include "SceneObject_Shape.h"
#include "Shapes/Shape.h"
#include "Material/Material.h"
#include "Medium/Medium.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"
#include "../Common/Reflection/Types/ReflectionSharedPtrType.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ShapeSceneObject)
{
    NFE_CLASS_PARENT(NFE::RT::ISceneObject);
    NFE_CLASS_MEMBER(mShape).NonNull();
    NFE_CLASS_MEMBER(mMedium);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

ShapeSceneObject::ShapeSceneObject(const ShapePtr& shape)
    : mShape(shape)
{
    NFE_ASSERT(mShape, "Invalid shape");
}

Box ShapeSceneObject::GetBoundingBox() const
{
    const Box localSpaceBox = mShape->GetBoundingBox();
    return { GetBaseTransform().TransformBox(localSpaceBox), GetTransform(1.0f).TransformBox(localSpaceBox) };
}

void ShapeSceneObject::BindMaterial(const MaterialPtr& material)
{
    mMaterial = material;
}

void ShapeSceneObject::BindMedium(const MediumPtr& medium)
{
    mMedium = medium;
}

void ShapeSceneObject::Traverse(const SingleTraversalContext& context, const uint32 objectID) const
{
    return mShape->Traverse(context, objectID);
}

bool ShapeSceneObject::Traverse_Shadow(const SingleTraversalContext& context, const uint32 objectID) const
{
    return mShape->Traverse_Shadow(context, objectID);
}

void ShapeSceneObject::Traverse(const PacketTraversalContext& context, const uint32 objectID, const uint32 numActiveGroups) const
{
    return mShape->Traverse(context, objectID, numActiveGroups);
}

void ShapeSceneObject::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const
{
    outIntersectionData.material = mMaterial.Get();
    outIntersectionData.medium = mMedium.Get();

    mShape->EvaluateIntersection(hitPoint, outIntersectionData);
}


} // namespace RT
} // namespace NFE

#include "PCH.h"
#include "SceneObject_Shape.h"
#include "Shapes/Shape.h"
#include "Rendering/ShadingData.h"
#include "Rendering/RenderingContext.h"
#include "Traversal/TraversalContext.h"
#include "Material/Material.h"
#include "../nfCommon/Math/Simd8Geometry.hpp"
#include "../nfCommon/Reflection/Types/ReflectionSharedPtrType.hpp"
#include "../nfCommon/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ShapeSceneObject)
{
    NFE_CLASS_PARENT(NFE::RT::ISceneObject);
    NFE_CLASS_MEMBER(mShape).NonNull();
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

void ShapeSceneObject::SetDefaultMaterial(const MaterialPtr& material)
{
    mDefaultMaterial = material;

    if (!mDefaultMaterial)
    {
        mDefaultMaterial = Material::GetDefaultMaterial();
    }
}

void ShapeSceneObject::Traverse(const SingleTraversalContext& context, const uint32 objectID) const
{
    return mShape->Traverse(context, objectID);
}

bool ShapeSceneObject::Traverse_Shadow(const SingleTraversalContext& context) const
{
    return mShape->Traverse_Shadow(context);
}

void ShapeSceneObject::Traverse(const PacketTraversalContext& context, const uint32 objectID, const uint32 numActiveGroups) const
{
    // TODO

    NFE_UNUSED(context);
    NFE_UNUSED(objectID);
    NFE_UNUSED(numActiveGroups);
}

void ShapeSceneObject::EvaluateIntersection(const HitPoint& hitPoint, IntersectionData& outIntersectionData) const
{
    outIntersectionData.material = mDefaultMaterial.Get();

    mShape->EvaluateIntersection(hitPoint, outIntersectionData);
}


} // namespace RT
} // namespace NFE

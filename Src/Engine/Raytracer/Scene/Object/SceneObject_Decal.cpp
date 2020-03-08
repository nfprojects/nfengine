#include "PCH.h"
#include "SceneObject_Decal.h"
#include "../../Rendering/ShadingData.h"
#include "../../Rendering/RenderingContext.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::DecalSceneObject)
{
    NFE_CLASS_PARENT(NFE::RT::ISceneObject);
    NFE_CLASS_MEMBER(baseColor);
    NFE_CLASS_MEMBER(roughness);
    NFE_CLASS_MEMBER(alphaMin);
    NFE_CLASS_MEMBER(alphaMax);
    NFE_CLASS_MEMBER(order);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

DecalSceneObject::DecalSceneObject() = default;

Box DecalSceneObject::GetBoundingBox() const
{
    const Box localSpaceBox(Vector4::Zero(), 1.0f);
    return { GetBaseTransform().TransformBox(localSpaceBox), GetTransform(1.0f).TransformBox(localSpaceBox) };
}

void DecalSceneObject::Apply(ShadingData& shadingData, RenderingContext& context) const
{
    Vector4 decalSpacePos = GetBaseInverseTransform().TransformPoint(shadingData.intersection.frame.GetTranslation());

    decalSpacePos = BipolarToUnipolar(decalSpacePos);

    if (decalSpacePos.x < 0.0f || decalSpacePos.y < 0.0f || decalSpacePos.z < 0.0f ||
        decalSpacePos.x > 1.0f || decalSpacePos.y > 1.0f || decalSpacePos.z > 1.0f)
    {
        return;
    }

    const float baseColorAlpha = 1.0f; // TODO
    const RayColor decalBaseColor = baseColor.Evaluate(decalSpacePos, context.wavelength);
    const float alpha = Lerp(alphaMin, alphaMax, baseColorAlpha);

    shadingData.materialParams.baseColor = RayColor::Lerp(shadingData.materialParams.baseColor, decalBaseColor, alpha);

    const float decalRoughness = roughness.Evaluate(decalSpacePos);
    shadingData.materialParams.roughness = Lerp(shadingData.materialParams.roughness, decalRoughness, alpha);
}

} // namespace RT
} // namespace NFE

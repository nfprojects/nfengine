#pragma once

#include "SceneObject.h"
#include "../../Material/MaterialParameter.h"

namespace NFE {
namespace RT {

enum class BlendingMode
{
    Alpha,
    Additive,
    Multiplicative,
};

class DecalSceneObject : public ISceneObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(DecalSceneObject);

public:
    NFE_RAYTRACER_API explicit DecalSceneObject();

    virtual Math::Box GetBoundingBox() const override;

    void Apply(ShadingData& shadingData, RenderingContext& context) const;

    ColorMaterialParameter baseColor;
    MaterialParameter roughness;

    float alphaMin = 0.0f;
    float alphaMax = 1.0f;

    uint32 order = 0;
};

} // namespace RT
} // namespace NFE

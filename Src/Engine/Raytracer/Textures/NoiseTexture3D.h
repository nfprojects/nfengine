#pragma once

#include "Texture.h"
#include "../Utils/Memory.h"
#include "../../Common/Math/HdrColor.hpp"

namespace NFE {
namespace RT {

// 3D simplex noise texture
class NFE_ALIGN(16) NoiseTexture3D
    : public ITexture
{
    NFE_DECLARE_POLYMORPHIC_CLASS(NoiseTexture3D)

public:
    NFE_RAYTRACER_API NoiseTexture3D(const Math::Vec4f& colorA, const Math::Vec4f& colorB, const uint32 numOctaves = 1);

    virtual const char* GetName() const override;
    virtual const Math::Vec4f Evaluate(const Math::Vec4f& coords) const override;
    virtual const Math::Vec4f Sample(const Math::Vec2f u, Math::Vec4f& outCoords, float* outPdf) const override;

    virtual bool OnPropertyChanged(const Common::StringView propertyName) override;

private:
    static constexpr float F3 = 0.33333333f;
    static constexpr float G3 = 0.16666667f;

    static float EvaluateInternal(const Math::Vec4f& coords);

    Math::HdrColorRGBA mColorA;
    Math::HdrColorRGBA mColorB;
    uint32 mNumOctaves;
    float mScale;
};

} // namespace RT
} // namespace NFE

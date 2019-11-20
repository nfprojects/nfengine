#pragma once

#include "Texture.h"
#include "../Utils/Memory.h"

namespace NFE {
namespace RT {

// 3D simplex noise texture
class NFE_ALIGN(16) NoiseTexture3D
    : public ITexture
{
public:
    NFE_RAYTRACER_API NoiseTexture3D(const Math::Vector4& colorA, const Math::Vector4& colorB, const uint32 numOctaves = 1);

    virtual const char* GetName() const override;
    virtual const Math::Vector4 Evaluate(const Math::Vector4& coords) const override;
    virtual const Math::Vector4 Sample(const Math::Float2 u, Math::Vector4& outCoords, float* outPdf) const override;

private:
    static constexpr float F3 = 0.33333333f;
    static constexpr float G3 = 0.16666667f;

    static float EvaluateInternal(const Math::Vector4& coords);

    Math::Vector4 mColorA;
    Math::Vector4 mColorB;
    uint32 mNumOctaves;
    float mScale;
};

} // namespace RT
} // namespace NFE

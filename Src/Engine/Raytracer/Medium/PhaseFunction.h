#pragma once

#include "../RayLib.h"
#include "../Utils/Memory.h"
#include "../../Common/Math/Vec4f.hpp"

namespace NFE {
namespace RT {

// generic phase function interface
class IPhaseFunction : public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(IPhaseFunction)
public:
    virtual ~IPhaseFunction() = default;
    virtual float Eval(const float cosTheta) const = 0;
    virtual float Sample(const Math::Vec4f& outDir, Math::Vec4f& inDir, const Math::Vec2f& u) const = 0;
};

// simplest isotropic phase function
class IsotropicPhaseFunction : public IPhaseFunction
{
    NFE_DECLARE_POLYMORPHIC_CLASS(IsotropicPhaseFunction)
public:
    virtual float Eval(const float cosTheta) const override;
    virtual float Sample(const Math::Vec4f& outDir, Math::Vec4f& inDir, const Math::Vec2f& u) const override;
};

// Henyey-Greenstein phase function
class HenyeyGreensteinPhaseFunction : public IPhaseFunction
{
    NFE_DECLARE_POLYMORPHIC_CLASS(HenyeyGreensteinPhaseFunction)
public:
    virtual float Eval(const float cosTheta) const override;
    virtual float Sample(const Math::Vec4f& outDir, Math::Vec4f& inDir, const Math::Vec2f& u) const override;
    float mAsymmetry = 0.0f;
};

} // namespace RT
} // namespace NFE

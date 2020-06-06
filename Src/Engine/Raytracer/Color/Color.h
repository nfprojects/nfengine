#pragma once

#include "RayColor.h"
#include "../../Common/Reflection/ReflectionClassDeclare.hpp"
#include "../../Common/Containers/SharedPtr.hpp"

namespace NFE {
namespace RT {

// Represents spectral power distribution (SPD)
class NFE_RAYTRACER_API IColor : public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(IColor)

public:
    virtual ~IColor() = default;

    // resolve to ray color, given ray's wavelength
    virtual const RayColor Resolve(const Wavelength& wavelength) const = 0;

    // check if color values are valid
    virtual bool IsValid() const = 0;
};

using ColorPtr = Common::SharedPtr<IColor>;


} // namespace RT
} // namespace NFE

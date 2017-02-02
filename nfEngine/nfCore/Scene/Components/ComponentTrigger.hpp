/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of trigger component class.
 */

#pragma once

#include "Component.hpp"

#include "nfCommon/Math/Box.hpp"


namespace NFE {
namespace Scene {

class CORE_API TriggerComponent : public IComponent
{
public:
    TriggerComponent();
    ~TriggerComponent();

private:
    // TODO temporary
    // there must be support for arbitrary shapes
    Math::Box mSize;
};

} // namespace Scene
} // namespace NFE

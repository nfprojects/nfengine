/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../Core.hpp"
#include "Component.hpp"

namespace NFE {
namespace Scene {

NFE_ALIGN16
class CORE_API TriggerComponent
    : public ComponentBase<TriggerComponent>
{
    // TODO

public:
    TriggerComponent();
    virtual ~TriggerComponent();
};

} // namespace Scene
} // namespace NFE

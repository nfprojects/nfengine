/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of input component class.
 */

#pragma once

#include "../../Core.hpp"
#include "Component.hpp"
#include "../../Input/InputEvent.hpp"

namespace NFE {
namespace Scene {


class CORE_API InputComponentInterface
{

};


/**
 * A component allowing entity for receiving input events.
 */
class CORE_API InputComponent
    : public Component
{
public:
    InputComponent();
    virtual ~InputComponent();

    /**
     * Called by the input system when event occurs.
     */
    bool OnEvent(const Input::Event& event);
};

} // namespace Scene
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of input component class.
 */

#pragma once

#include "Component.hpp"
#include "../../Input/InputEvent.hpp"


namespace NFE {
namespace Scene {

/**
 * A component allowing an entity for receiving input events.
 */
class CORE_API InputComponent
    : public IComponent
{
public:
    InputComponent();
    virtual ~InputComponent();

    /**
     * Enable event receiving.
     */
    void Enable();

    /**
     * Disable event receiving.
     */
    void Disable();

private:
    // IComponent interface
    void OnAttach() override;
    void OnDetach() override;

    // register/unregister in the input system
    void Register();
    void Unregister();

    bool mEnabled;
};

} // namespace Scene
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of controller component class.
 */

#pragma once

#include "Component.hpp"
#include "../../Input/InputEvent.hpp"


namespace NFE {
namespace Scene {

/**
 * A component allowing an entity for receiving events.
 */
class CORE_API ControllerComponent : public IComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ControllerComponent)
    NFE_MAKE_NONCOPYABLE(ControllerComponent)

public:
    ControllerComponent();
    ControllerComponent(ControllerComponent&&) = default;
    ControllerComponent& operator = (ControllerComponent&&) = default;

    virtual ~ControllerComponent();

    /**
     * Set the controller object.
     */
    void SetController(EntityControllerPtr&& controller);

    /**
     * Destroy owned controller.
     */
    void ClearController();

    IEntityController* GetController() const { return mController.Get(); }

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

    // register/unregister in the event system
    void AttachController();
    void DetachController();

    // controller object itself
    EntityControllerPtr mController;

    // TODO event filtering

    bool mEnabled;
};

} // namespace Scene
} // namespace NFE

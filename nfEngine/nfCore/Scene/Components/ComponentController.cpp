/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of controller component class.
 */

#include "PCH.hpp"
#include "ComponentController.hpp"
#include "../Entity.hpp"
#include "../EntityController.hpp"
#include "../SceneManager.hpp"
#include "../Systems/EventSystem.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::ControllerComponent)
    NFE_CLASS_PARENT(NFE::Scene::IComponent)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

ControllerComponent::ControllerComponent()
    : mEnabled(true)
{
}

ControllerComponent::~ControllerComponent()
{
}

void ControllerComponent::OnAttach()
{
    if (mEnabled)
    {
        AttachController();
    }
}

void ControllerComponent::OnDetach()
{
    if (mEnabled)
    {
        DetachController();
    }
}

void ControllerComponent::SetController(EntityControllerPtr&& controller)
{
    const bool wasEnabled = mEnabled;

    // make sure the controller is unregistered from the event system
    Disable();

    // set controller
    mController = std::move(controller);

    // enable if it was enabled before
    if (wasEnabled)
    {
        Enable();
    }
}

void ControllerComponent::ClearController()
{
    if (mController)
    {
        DetachController();
        mController.Reset();
    }
}

void ControllerComponent::Enable()
{
    if (mEnabled)
    {
        // nothing to do
        return;
    }

    mEnabled = true;
    AttachController();
}

void ControllerComponent::Disable()
{
    if (!mEnabled)
    {
        // nothing to do
        return;
    }

    mEnabled = false;
    DetachController();
}

void ControllerComponent::AttachController()
{
    if (!mController)
    {
        return;
    }

    Entity* entity = GetEntity();
    if (entity)
    {
        EventSystem* system = entity->GetScene().GetSystem<EventSystem>();
        NFE_ASSERT(system, "Invalid Event System");

        mController->Attach(entity);
        system->RegisterEntityController(mController.Get());
    }
}

void ControllerComponent::DetachController()
{
    if (!mController)
    {
        return;
    }

    Entity* entity = GetEntity();
    if (entity)
    {
        EventSystem* system = GetScene().GetSystem<EventSystem>();
        NFE_ASSERT(system, "Invalid Event System");

        mController->Detach();
        system->UnregisterEntityController(mController.Get());
    }
}


} // namespace Scene
} // namespace NFE

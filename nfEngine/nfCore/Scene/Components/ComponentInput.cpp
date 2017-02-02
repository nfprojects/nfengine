/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of input component class.
 */

#include "PCH.hpp"
#include "ComponentInput.hpp"
#include "Engine.hpp"
#include "../Entity.hpp"
#include "../SceneManager.hpp"
#include "../Systems/InputSystem.hpp"


namespace NFE {
namespace Scene {

InputComponent::InputComponent()
    : mEnabled(false)
{
}

InputComponent::~InputComponent()
{
}

void InputComponent::OnAttach()
{
    if (mEnabled)
    {
        Register();
    }
}

void InputComponent::OnDetach()
{
    if (mEnabled)
    {
        Unregister();
    }
}

void InputComponent::Enable()
{
    if (mEnabled)
    {
        // nothing to do
        return;
    }

    mEnabled = true;
    Register();
}

void InputComponent::Disable()
{
    if (!mEnabled)
    {
        // nothing to do
        return;
    }

    mEnabled = false;
    Unregister();
}

void InputComponent::Register()
{
    Entity* entity = GetEntity();
    if (entity)
    {
        SceneManager* scene = entity->GetScene();
        NFE_ASSERT(scene, "Entity not attached to a scene");

        InputSystem* system = scene->GetInputSystem();
        NFE_ASSERT(system, "Invalid Input System");

        system->RegisterComponent(this);
    }
}

void InputComponent::Unregister()
{
    Entity* entity = GetEntity();
    if (entity)
    {
        SceneManager* scene = entity->GetScene();
        NFE_ASSERT(scene, "Entity not attached to a scene");

        InputSystem* system = scene->GetInputSystem();
        NFE_ASSERT(system, "Invalid Input System");

        system->UnregisterComponent(this);
    }
}


} // namespace Scene
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of input system.
 */

#include "PCH.hpp"
#include "InputSystem.hpp"
#include "EntitySystem.hpp"
#include "../Components/ComponentInput.hpp"
#include "../EntityController.hpp"
#include "../Events/Event_Input.hpp"

#include "nfCommon/System/Assertion.hpp"


namespace NFE {
namespace Scene {

InputSystem::InputSystem(SceneManager* scene)
    : ISystem(scene)
    , mScene(scene)
{
}

InputSystem::~InputSystem()
{
    NFE_ASSERT(mRegisteredComponents.empty(), "There are still registered input components. Memory leak?");
}

void InputSystem::Update(float dt)
{
    // TODO this is not the best way...
    // there should be "EventSystem" responsible for dispatching all the events
    for (const InputComponent* component : mRegisteredComponents)
    {
        const Entity* entity = component->GetEntity();
        NFE_ASSERT(entity, "Unattached input component");

        IEntityController* controller = entity->GetController();
        if (!controller)
            continue;

        for (const Input::EventData& data : mEvents)
        {
            const Event_Input inputEvent(data);
            controller->OnEvent(inputEvent);
        }
    }

    mEvents.clear();
}

void InputSystem::PushEvent(const Input::EventData& event)
{
    mEvents.push_back(event);
}

void InputSystem::RegisterComponent(const InputComponent* component)
{
    mRegisteredComponents.insert(component);
}

void InputSystem::UnregisterComponent(const InputComponent* component)
{
    mRegisteredComponents.erase(component);
}

} // namespace Scene
} // namespace NFE

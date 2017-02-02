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

using namespace Common;

void InputSystem::InputAxis::ClampToRange()
{
    if (wrap)
    {
        const float span = max - min;
        while (value > max) value -= span;
        while (value < min) value += span;
    }
    else
    {
        value = Math::Clamp(value, min, max);
    }
}


InputSystem::InputSystem(SceneManager* scene)
    : ISystem(scene)
    , mScene(scene)
{
}

InputSystem::~InputSystem()
{
    NFE_ASSERT(mRegisteredComponents.empty(), "There are still registered input components. Memory leak?");
}

bool InputSystem::RegisterAxis(const char* name, float min, float max, bool wrap)
{
    mAxes.PushBack(InputAxis(StringView(name), min, max, wrap));
    return true;
}

bool InputSystem::SetAxisValue(const char* name, float newValue, bool offset)
{
    for (InputAxis& axis : mAxes)
    {
        if (axis.name == name)
        {
            float& value = axis.value;
            const float oldValue = axis.value;

            // update
            if (offset)
            {
                value += newValue;
            }
            else
            {
                value = newValue;
            }

            axis.ClampToRange();

            if (oldValue != value)
            {
                PushEvent(Input::EventData(Input::EventData::Type::Axis, StringView(name), value));
            }

            return true;
        }
    }

    LOG_ERROR("Invalid input axis name");
    return false;
}

float InputSystem::GetAxisValue(const char* name) const
{
    for (const InputAxis& axis : mAxes)
    {
        if (axis.name == name)
        {
            return axis.value;
        }
    }

    LOG_ERROR("Invalid input axis name");
    return 0.0f;
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

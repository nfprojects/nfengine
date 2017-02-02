/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of input system.
 */

#include "PCH.hpp"
#include "InputSystem.hpp"
#include "EventSystem.hpp"
#include "../Events/Event_Input.hpp"
#include "../SceneManager.hpp"

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
    EventSystem* eventSystem = mScene->GetEventSystem();
    NFE_ASSERT(eventSystem, "Invalid event system");

    for (const Input::EventData& data : mEvents)
    {
        EventPtr inputEvent = MakeUniquePtr<Event_Input>(data);
        eventSystem->Broadcast(std::move(inputEvent));
    }

    mEvents.clear();
}

void InputSystem::PushEvent(const Input::EventData& event)
{
    mEvents.push_back(event);
}

} // namespace Scene
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of input system.
 */

#include "PCH.hpp"
#include "InputSystem.hpp"
#include "Engine.hpp"


namespace NFE {
namespace Scene {

InputSystem::InputSystem(SceneManager* scene)
    : ISystem(scene)
{

}

void InputSystem::Update(float dt)
{
    // TODO send the event to all the input components

    mEvents.clear();
}

void InputSystem::PushEvent(const Input::Event& event)
{
    mEvents.push_back(event);
}

} // namespace Scene
} // namespace NFE

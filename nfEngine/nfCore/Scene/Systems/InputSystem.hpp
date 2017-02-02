/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"
#include "../../Input/InputEvent.hpp"

#include <unordered_set>


namespace NFE {
namespace Scene {

/**
 * System redirecting input events to components.
 */
class CORE_API InputSystem final
    : public ISystem
{
    NFE_MAKE_NONCOPYABLE(InputSystem);

public:
    InputSystem(SceneManager* scene);
    ~InputSystem();

    /**
     * Update the system.
     */
    void Update(float dt) override;

    /**
     * Enqueue an event.
     * @note    This is temporary.
     */
    void PushEvent(const Input::EventData& event);

    void RegisterComponent(const InputComponent* component);
    void UnregisterComponent(const InputComponent* component);

private:
    SceneManager* mScene;

    // queued events
    std::vector<Input::EventData> mEvents;

    // registered components that parent entities will receive input events
    // TODO event filtering per component
    std::unordered_set<const InputComponent*> mRegisteredComponents;
};

} // namespace Scene
} // namespace NFE

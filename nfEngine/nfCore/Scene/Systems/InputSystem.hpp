/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"
#include "../../Input/InputEvent.hpp"

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

    /**
     * Update the system.
     */
    void Update(float dt) override;

    /**
     * Enqueue an event.
     * @note    This is temporary.
     */
    void PushEvent(const Input::Event& event);

private:
    std::vector<Input::Event> mEvents;
};

} // namespace Scene
} // namespace NFE

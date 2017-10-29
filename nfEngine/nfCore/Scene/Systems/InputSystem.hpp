/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"
#include "../../Input/InputEvent.hpp"

#include "nfCommon/Containers/String.hpp"
#include "nfCommon/Containers/DynArray.hpp"


namespace NFE {
namespace Scene {

/**
 * System redirecting input events to entities.
 */
class CORE_API InputSystem final
    : public ISystem
{
    NFE_DECLARE_POLYMORPHIC_CLASS(InputSystem)
    NFE_MAKE_NONCOPYABLE(InputSystem);

public:
    static const int ID = 2;

    explicit InputSystem(SceneManager& scene);
    ~InputSystem();

    /**
     * Update the system.
     */
    void Update(const SystemUpdateContext& context) override;

    /**
     * Enqueue an event.
     * @note    This is temporary.
     */
    void PushEvent(const Input::EventData& event);
    void PushEvent(Input::EventData&& event);

    /**
     * Register input axis.
     */
    bool RegisterAxis(const char* name, float min, float max, bool wrap);

    /**
     * Update axis value.
     */
    bool SetAxisValue(const char* name, float newValue, bool offset = false);

    /**
     * Get current axis value.
     */
    float GetAxisValue(const char* name) const;

private:
    struct InputAxis
    {
        Common::String name;
        float value;
        float min;
        float max;
        bool wrap;

        InputAxis(Common::StringView name, float min, float max, bool wrap)
            : name(name), value(0.0f), min(min), max(max), wrap(wrap)
        {}

        void ClampToRange();
    };

    // TODO this should be probably moved to some global "input manager"
    Common::DynArray<InputAxis> mAxes;

    // queued events
    Common::DynArray<Input::EventData> mEvents;
};

} // namespace Scene
} // namespace NFE

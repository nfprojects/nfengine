/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"
#include "../Events/Event.hpp"

#include "nfCommon/Containers/DynArray.hpp"

#include <unordered_set>


namespace NFE {
namespace Scene {

/**
 * System responsible for events dispatching.
 */
class CORE_API EventSystem final
    : public ISystem
{
    NFE_MAKE_NONCOPYABLE(EventSystem);

public:
    EventSystem(SceneManager* scene);
    ~EventSystem();

    /**
     * Register an entity controller in the system.
     */
    void RegisterEntityController(IEntityController* entityController);

    /**
     * Unregister an entity controller from the system.
     */
    void UnregisterEntityController(IEntityController* entityController);

    /**
     * Update the system.
     */
    void Update(float dt) override;

    /**
     * Send an event to specific entity.
     */
    void Send(EventPtr event, const Entity& target);

    /**
     * Send an event to all the entities.
     */
    void Broadcast(EventPtr event);

    // TODO send to entities in range, etc.

private:
    struct EventData
    {
        EventPtr event;
        IEntityController* target;

        EventData(EventPtr event, IEntityController* target)
            : event(std::move(event))
            , target(target)
        { }
    };

    Common::DynArray<EventData> mEventsQueue;
    Common::DynArray<EventPtr> mBroadcastEventsQueue;

    // registered entity controllers that will receive input events
    // TODO event filtering per entity
    std::unordered_set<IEntityController*> mRegisteredControllers;
};

} // namespace Scene
} // namespace NFE

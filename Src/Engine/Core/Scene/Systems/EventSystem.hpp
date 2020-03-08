/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"
#include "../Events/Event.hpp"

#include "../../../Common/Containers/DynArray.hpp"
#include "../../../Common/Containers/HashSet.hpp"


namespace NFE {
namespace Scene {

/**
 * System responsible for events dispatching.
 */
class CORE_API EventSystem final
    : public ISystem
{
    NFE_DECLARE_POLYMORPHIC_CLASS(EventSystem)
    NFE_MAKE_NONCOPYABLE(EventSystem);

public:
    static const int ID = 1;

    explicit EventSystem(Scene& scene);
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
    void Update(const SystemUpdateContext& context) override;

    /**
     * Send an event to specific entity.
     */
    void Send(const EventPtr& event, const Entity& target);

    /**
     * Send an event to all the entities.
     */
    void Broadcast(const EventPtr& event);
    void Broadcast(EventPtr&& event);

    // TODO send to entities in range, etc.

private:
    struct EventData
    {
        const EventPtr event;
        IEntityController* target;

        EventData(const EventPtr& event, IEntityController* target)
            : event(event)
            , target(target)
        { }
    };

    Common::DynArray<EventData> mEventsQueue;
    Common::DynArray<EventPtr> mBroadcastEventsQueue;

    // registered entity controllers that will receive input events
    // TODO event filtering per entity
    Common::HashSet<IEntityController*> mRegisteredControllers;
};

} // namespace Scene
} // namespace NFE

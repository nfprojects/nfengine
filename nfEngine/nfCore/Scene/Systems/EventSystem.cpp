/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of event system.
 */

#include "PCH.hpp"
#include "EventSystem.hpp"
#include "EntitySystem.hpp"
#include "../EntityController.hpp"
#include "../Components/ComponentController.hpp"

#include "nfCommon/System/Assertion.hpp"


namespace NFE {
namespace Scene {

using namespace Common;

EventSystem::EventSystem(SceneManager* scene)
    : ISystem(scene)
{
}

EventSystem::~EventSystem()
{
    NFE_ASSERT(mEventsQueue.Empty(), "Events queue should be empty");
    NFE_ASSERT(mBroadcastEventsQueue.Empty(), "Broadcast events queue should be empty");
    NFE_ASSERT(mRegisteredControllers.empty(), "There are still entity controllers registered in the envet system");
}

void EventSystem::RegisterEntityController(IEntityController* controller)
{
    NFE_ASSERT(mRegisteredControllers.count(controller) == 0, "Entity controller is already registered");
    mRegisteredControllers.insert(controller);
}

void EventSystem::UnregisterEntityController(IEntityController* controller)
{
    NFE_ASSERT(mRegisteredControllers.count(controller) == 1, "Entity controller is not registered");
    mRegisteredControllers.erase(controller);
}

void EventSystem::Send(EventPtr event, const Entity& target)
{
    const ControllerComponent* component = target.GetComponent<ControllerComponent>();
    if (!component)
    {
        // entity without controller component cannot receive events
        return;
    }

    IEntityController* targetController = component->GetController();
    if (!targetController)
    {
        return;
    }

    EventData data(std::move(event), targetController);
    mEventsQueue.PushBack(std::move(data));
}

void EventSystem::Broadcast(EventPtr event)
{
    mBroadcastEventsQueue.PushBack(std::move(event));
}

void EventSystem::Update(float dt)
{
    // TODO multithreaded processing !!!


    // process single-target events
    {
        for (const EventData& eventData : mEventsQueue)
        {
            if (mRegisteredControllers.count(eventData.target) == 0)
            {
                continue;
            }

            const Event& event = *eventData.event;
            eventData.target->OnEvent(event);
        }

        mEventsQueue.Clear();
    }

    // process broadcast events
    {
        for (const EventPtr& event : mBroadcastEventsQueue)
        {
            for (IEntityController* controller : mRegisteredControllers)
            {
                controller->OnEvent(*event);
            }
        }

        mBroadcastEventsQueue.Clear();
    }
}

} // namespace Scene
} // namespace NFE

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
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::EventSystem)
    NFE_CLASS_PARENT(NFE::Scene::ISystem)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

using namespace Common;

EventSystem::EventSystem(Scene& scene)
    : ISystem(scene)
{
}

EventSystem::~EventSystem()
{
    NFE_ASSERT(mEventsQueue.Empty(), "Events queue should be empty");
    NFE_ASSERT(mBroadcastEventsQueue.Empty(), "Broadcast events queue should be empty");
    NFE_ASSERT(mRegisteredControllers.Empty(), "There are still entity controllers registered in the envet system");
}

void EventSystem::RegisterEntityController(IEntityController* controller)
{
    NFE_ASSERT(!mRegisteredControllers.Exists(controller), "Entity controller is already registered");
    mRegisteredControllers.Insert(controller);
}

void EventSystem::UnregisterEntityController(IEntityController* controller)
{
    NFE_ASSERT(mRegisteredControllers.Exists(controller), "Entity controller is not registered");
    mRegisteredControllers.Erase(controller);
}

void EventSystem::Send(const EventPtr& event, const Entity& target)
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

    EventData data(event, targetController);
    mEventsQueue.PushBack(std::move(data));
}

void EventSystem::Broadcast(const EventPtr& event)
{
    mBroadcastEventsQueue.PushBack(event);
}

void EventSystem::Broadcast(EventPtr&& event)
{
    mBroadcastEventsQueue.PushBack(std::move(event));
}

void EventSystem::Update(const SystemUpdateContext& context)
{
    NFE_UNUSED(context);
    // TODO multithreaded processing !!!

    // process single-target events
    {
        for (const EventData& eventData : mEventsQueue)
        {
            if (!mRegisteredControllers.Exists(eventData.target))
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

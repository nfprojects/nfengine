/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of trigger system.
 */

#include "PCH.hpp"
#include "TriggerSystem.hpp"
#include "Engine.hpp"
#include "EventSystem.hpp"
#include "../SceneManager.hpp"
#include "../Events/Event_Trigger.hpp"

namespace NFE {
namespace Scene {

using namespace Math;


TriggerSystem::TriggerSystem(SceneManager& scene)
    : ISystem(scene)
    , mTriggerID(0)
{ }

TriggerSystem::~TriggerSystem()
{
    NFE_ASSERT(mTriggers.Size() == 0,
               "Destroying trigger system while there are some registered triggers left. This may indicate a memory leak");
}

TriggerID TriggerSystem::CreateTrigger(const TriggerDesc& desc)
{
    // generate trigger ID
    const TriggerID id = ++mTriggerID;

    // insert to BVH
    void* userData = reinterpret_cast<void*>(static_cast<size_t>(id));
    const uint32 treeNode = mTriggersTree.Insert(desc.box, userData);

    TriggerObject object;
    object.box = desc.box;
    object.entity = desc.entity;
    object.id = id;
    object.treeNodeId = treeNode;
    object.isTarget = desc.isTarget;

    if (mTriggers.Insert(id, object).iterator == mTriggers.End())
    {
        NFE_LOG_ERROR("Failed to create trigger object");
        return 0;
    }

    // we must find overlapping pairs in this volume
    InvalidateArea(desc.box);

    return id;
}

void TriggerSystem::UpdateTrigger(const TriggerID id, const Math::Box& newBox)
{
    TriggerObject& obj = mTriggers[id];
    NFE_ASSERT(obj.id == id, "Trigger ID is corrupted");

    if (obj.box != newBox)
    {
        InvalidateArea(obj.box);
        InvalidateArea(newBox);
        obj.box = newBox;

        mInvalidatedTriggerIDs.Insert(id);
        mTriggersTree.Move(obj.treeNodeId, newBox);
    }
}

void TriggerSystem::DestroyTrigger(const TriggerID id)
{
    const auto iter = mTriggers.Find(id);
    NFE_ASSERT(iter != mTriggers.End(), "Invalid trigger ID");

    const TriggerObject& obj = (*iter).second;
    NFE_ASSERT(obj.id == id, "Trigger ID is corrupted");

    InvalidateArea(obj.box);

    mTriggersTree.Remove(obj.treeNodeId);
    mTriggers.Erase(iter);
}

void TriggerSystem::InvalidateArea(const Math::Box& box)
{
    mTriggersTree.Query(box, [this](void* userData)
    {
        TriggerID triggerID = static_cast<TriggerID>(reinterpret_cast<size_t>(userData));
        mInvalidatedTriggerIDs.Insert(triggerID);
    });
}

void TriggerSystem::Update(const SystemUpdateContext& context)
{
    NFE_UNUSED(context);

    // Resolve ID to actual trigger object pointers in order to reduce number of Map accesses
    // from N^2 to N.
    Common::DynArray<const TriggerObject*> invalidatedTriggers;
    {
        invalidatedTriggers.Reserve(mInvalidatedTriggerIDs.Size());
        for (const TriggerID id : mInvalidatedTriggerIDs)
        {
            const TriggerObject& obj = mTriggers[id];
            invalidatedTriggers.PushBack(&obj);

            NFE_ASSERT(obj.id == id, "Trigger ID is corrupted");
        }
        mInvalidatedTriggerIDs.Clear();
    }

    EventSystem* eventSystem = GetScene().GetSystem<EventSystem>();
    NFE_ASSERT(eventSystem, "Invalid event system");

    // Discover current interactions within invalidated set and generate events
    for (const TriggerObject* triggerA : invalidatedTriggers)
    {
        // trigger A is source
        if (triggerA->isTarget)
            continue;

        for (const TriggerObject* triggerB : invalidatedTriggers)
        {
            // triggerB is target
            if (!triggerB->isTarget)
                continue;

            TriggerInteraction interaction;
            interaction.triggerA = triggerA->id;
            interaction.triggerB = triggerB->id;

            EventPtr event;

            if (Math::Intersect(triggerA->box, triggerB->box))
            {
                const auto result = mInteractions.InsertOrReplace(interaction);
                if (!result.replaced)
                {
                    // triggers were not interacting before
                    event = Common::MakeUniquePtr<Event_Trigger>(*triggerA->entity, *triggerB->entity, Event_Trigger::Type::Enter);
                    NFE_LOG_DEBUG("Entering trigger target=%u source=%u", interaction.triggerA, interaction.triggerB);
                }
            }
            else // no interaction
            {
                if (mInteractions.Erase(interaction))
                {
                    // triggers were interacting before
                    event = Common::MakeUniquePtr<Event_Trigger>(*triggerA->entity, *triggerB->entity, Event_Trigger::Type::Leave);
                    NFE_LOG_DEBUG("Leaving trigger target=%u source=%u", interaction.triggerA, interaction.triggerB);
                }
            }

            if (event)
            {
                eventSystem->Send(event, *triggerA->entity);
                eventSystem->Send(event, *triggerB->entity);
            }
        }
    }
}

} // namespace Scene
} // namespace NFE

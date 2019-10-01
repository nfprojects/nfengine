/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of entity system.
 */

#include "PCH.hpp"
#include "EntitySystem.hpp"
#include "../Scene.hpp"
#include "../Systems/EventSystem.hpp"
#include "../Events/Event_Tick.hpp"



NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::EntitySystem)
    NFE_CLASS_PARENT(NFE::Scene::ISystem)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

using namespace Common;
using namespace Math;

EntitySystem::EntitySystem(Scene& scene)
    : ISystem(scene)
    , mFrameNumber(0)
{

}

Entity* EntitySystem::CreateEntity(Entity* parent)
{
    // allocate entity ID
    bool allocateNewPointer = true;
    EntityID entityID = static_cast<EntityID>(mAllEntities.Size());
    if (!mFreeEntities.Empty())
    {
        allocateNewPointer = false;
        entityID = mFreeEntities.Back();
        mFreeEntities.PopBack();
    }

    auto newEntity = MakeUniquePtr<Entity>(entityID, GetScene(), parent);
    Entity* entityPointer = newEntity.Get();

    if (allocateNewPointer)
    {
        mAllEntities.PushBack(std::move(newEntity));
    }
    else
    {
        mAllEntities[entityID] = std::move(newEntity);
    }

    return entityPointer;
}

void EntitySystem::RemoveEntity(Entity* entity)
{
    NFE_ASSERT(entity, "Invalid entity pointer");
    if (!entity)
    {
        return;
    }

    const EntityID id = entity->GetID();
    NFE_ASSERT(id < mAllEntities.Size(), "Entity ID corrupted");
    NFE_ASSERT(mAllEntities[id].Get() == entity, "Entity ID corrupted");

    mAllEntities[id].Reset();
    mFreeEntities.PushBack(id);
}

void EntitySystem::RemoveAllEntities()
{
    mAllEntities.Clear();
}

void EntitySystem::Update(const SystemUpdateContext& context)
{
    EventSystem* eventSystem = GetScene().GetSystem<EventSystem>();
    NFE_ASSERT(eventSystem, "Invalid event system");

    // broadcast "tick" event
    auto tickEvent = MakeSharedPtr<Event_Tick>(context.timeDelta, mFrameNumber++);
    eventSystem->Broadcast(tickEvent);

    RemovePendingEntities();
    UpdateEntities();
}

void EntitySystem::OnMarkEntityAsDirty(Entity* entity, int flag)
{
    if (NFE_ENTITY_FLAG_TO_REMOVE == flag)
    {
        mEntitiesToRemove.Insert(entity);
    }

    if (NFE_ENTITY_FLAG_MOVED_LOCAL == flag || NFE_ENTITY_FLAG_MOVED_GLOBAL == flag)
    {
        mEntitiesToUpdate.Insert(entity);
    }
}

void EntitySystem::RemovePendingEntities()
{
    for (Entity* entityToRemove : mEntitiesToRemove)
    {
        RemoveEntity(entityToRemove);
    }

    mEntitiesToRemove.Clear();
}

void EntitySystem::UpdateEntities()
{
    // TODO sort by entity depth (parent entities must be updated first)
    // TODO parallelize

    for (Entity* entityToUpdate : mEntitiesToUpdate)
    {
        entityToUpdate->Update();
    }

    mEntitiesToUpdate.Clear();
    mEntitiesToRemove.Clear();
}

} // namespace Scene
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of entity system.
 */

#include "PCH.hpp"
#include "EntitySystem.hpp"
#include "Engine.hpp"
#include "../Systems/EventSystem.hpp"
#include "../Events/Event_Tick.hpp"
#include "Utils/ConfigVariable.hpp"

#include "nfCommon/System/Assertion.hpp"


namespace NFE {
namespace Scene {

using namespace Common;
using namespace Math;

EntitySystem::EntitySystem(SceneManager* scene)
    : ISystem(scene)
    , mFrameNumber(0)
{

}

Entity* EntitySystem::CreateEntity(Entity* parent)
{
    // allocate entity ID
    bool allocateNewPointer = true;
    EntityID entityID = static_cast<EntityID>(mAllEntities.size());
    if (!mFreeEntities.empty())
    {
        allocateNewPointer = false;
        entityID = mFreeEntities.back();
        mFreeEntities.pop_back();
    }

    auto newEntity = MakeUniquePtr<Entity>(entityID, GetScene(), parent);
    Entity* entityPointer = newEntity.Get();

    if (allocateNewPointer)
    {
        mAllEntities.push_back(std::move(newEntity));
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
    NFE_ASSERT(id < mAllEntities.size(), "Entity ID corrupted");
    NFE_ASSERT(mAllEntities[id].Get() == entity, "Entity ID corrupted");

    mAllEntities[id].Reset();
    mFreeEntities.push_back(id);
}

void EntitySystem::RemoveAllEntities()
{
    mAllEntities.clear();
}

void EntitySystem::Update(float dt)
{
    EventSystem* eventSystem = mScene->GetEventSystem();
    NFE_ASSERT(eventSystem, "Invalid event system");

    // broadcast "tick" event
    auto tickEvent = MakeUniquePtr<Event_Tick>(dt, mFrameNumber++);
    eventSystem->Broadcast(tickEvent);

    RemovePendingEntities();
    UpdateEntities();
}

void EntitySystem::OnMarkEntityAsDirty(Entity* entity, int flag)
{
    if (NFE_ENTITY_FLAG_TO_REMOVE == flag)
    {
        mEntitiesToRemove.insert(entity);
    }

    if (NFE_ENTITY_FLAG_MOVED_LOCAL == flag || NFE_ENTITY_FLAG_MOVED_GLOBAL == flag)
    {
        mEntitiesToUpdate.insert(entity);
    }
}

void EntitySystem::RemovePendingEntities()
{
    for (Entity* entityToRemove : mEntitiesToRemove)
    {
        RemoveEntity(entityToRemove);
    }

    mEntitiesToRemove.clear();
}

void EntitySystem::UpdateEntities()
{
    // TODO sort by entity depth (parent entities must be updated first)
    // TODO parallelize

    for (Entity* entityToUpdate : mEntitiesToUpdate)
    {
        entityToUpdate->Update();
    }

    mEntitiesToUpdate.clear();
    mEntitiesToRemove.clear();
}

} // namespace Scene
} // namespace NFE

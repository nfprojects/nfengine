/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of entity system.
 */

#include "PCH.hpp"
#include "EntitySystem.hpp"
#include "Engine.hpp"
#include "Utils/ConfigVariable.hpp"

#include "nfCommon/Assertion.hpp"


namespace NFE {
namespace Scene {

using namespace Math;

EntitySystem::EntitySystem(SceneManager* scene)
    : ISystem(scene)
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

    std::unique_ptr<Entity> newEntity(new Entity(entityID, GetScene(), parent));
    Entity* entityPointer = newEntity.get();

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
    NFE_ASSERT(mAllEntities[id].get() == entity, "Entity ID corrupted");

    mAllEntities[id].reset();
    mFreeEntities.push_back(id);
}

void EntitySystem::RemovePendingEntities()
{
    for (Entity* entityToRemove : mEntitiesToRemove)
    {
        RemoveEntity(entityToRemove);
    }

    mEntitiesToRemove.clear();
}

void EntitySystem::Update(float dt)
{
    RemovePendingEntities();
}

void EntitySystem::OnMarkEntityAsDirty(Entity* entity, int flag)
{
    if (NFE_ENTITY_FLAG_TO_REMOVE == flag)
    {
        mEntitiesToRemove.insert(entity);
    }
}

} // namespace Scene
} // namespace NFE

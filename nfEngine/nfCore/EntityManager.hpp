/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Scene class.
 */

#pragma once

#include "Core.hpp"
#include "Entity.hpp"
#include "Components/Component.hpp"

namespace NFE {
namespace Scene {

typedef uint32 EntityID;

/**
 * Entity manager.
 */
class CORE_API EntityManager
{
    // entity id counter
    static EntityID gLastEntityId;

    std::set<EntityID> mEntities;

public:
    /**
     * Create an new empty entity.
     */
    EntityID CreateEntity();

    /**
     * Remove an entity immediately.
     * @return 'true' if entity existed before the method call.
     */
    bool RemoveEntity(EntityID entity);

    /**
     * Check if an entity with given ID exists.
     */
    bool EntityExists(EntityID entity) const;


    /**
     * Add a component to an entity.
     * @param entity    Entity ID
     * @param component Component data
     */
    template <typename CompType>
    bool AddComponent(EntityID entity, CompType&& component);

    /**
     * Remove a component from an entity immediately.
     */
    template <typename CompType>
    bool RemoveComponent(EntityID entity);

    /**
     * Get an entity component.
     * Notice: the returned pointer is only valid proir to next entity/component
     * addition/removal.
     */
    template <typename CompType>
    CompType* GetComponent(EntityID entity) const;

    /**
     * Iterate through entities that has specific components assigned.
     * Used by system mostly.
     */
    template <typename  ...CompTypes>
    void ForEach(std::function<void(EntityID entity, CompTypes& ...components)> func) const;
};


template <typename CompType>
bool EntityManager::AddComponent(EntityID entity, CompType&& component)
{
    // TODO

    (void)entity;
    (void)component;
    return true;
}

template <typename CompType>
bool EntityManager::RemoveComponent(EntityID entity)
{
    // TODO

    return true;
}

template <typename CompType>
CompType* EntityManager::GetComponent(EntityID entity) const
{
    // TODO

    return nullptr;
}

template <typename  ...CompTypes>
void EntityManager::ForEach(std::function<void(EntityID entity,
                                               CompTypes& ...components)> func) const
{
    // TODO
}

} // namespace Scene
} // namespace NFE

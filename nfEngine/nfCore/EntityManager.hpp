/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Scene class.
 */

#pragma once

#include "Core.hpp"
#include "Components/Component.hpp"

namespace NFE {
namespace Scene {

typedef uint32 EntityID;

/**
 * Entity manager.
 */
class CORE_API EntityManager final
{
    // entity id counter
    static EntityID gLastEntityId;

    // TODO: this is temporary
    std::set<EntityID> mEntities;
    std::map<EntityID, Component*> mComponents[NFE_MAX_COMPONENT_TYPES];

public:
    ~EntityManager();

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
    bool AddComponent(EntityID entity, const CompType& component);

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
    template <typename CompType>
    void ForEach(std::function<void(EntityID entity, CompType* component)> func);

    template <typename CompTypeA, typename CompTypeB>
    void ForEach(std::function<void(EntityID entity, CompTypeA* componentA, CompTypeB* componentB)> func);
};


template <typename CompType>
bool EntityManager::AddComponent(EntityID entity, const CompType& component)
{
    int compId = Component::GetID<CompType>();
    assert(compId >= 0);
    assert(compId < NFE_MAX_COMPONENT_TYPES);

    bool newComponent = true;
    auto& componentsMap = mComponents[compId];
    auto it = componentsMap.find(entity);
    newComponent = (it == componentsMap.end());
    if (!newComponent)
        RemoveComponent<CompType>(entity);

    CompType* ptr = new CompType(component); // TODO: temporary
    componentsMap[entity] = ptr;

    return newComponent;
}

template <typename CompType>
bool EntityManager::RemoveComponent(EntityID entity)
{
    int compId = Component::GetID<CompType>();
    assert(compId >= 0);
    assert(compId < NFE_MAX_COMPONENT_TYPES);

    auto& componentsMap = mComponents[compId];
    auto it = componentsMap.find(entity);

    if (it != componentsMap.end())
    {
        CompType* ptr = static_cast<CompType*>(it->second);
        if (ptr != nullptr)
            delete ptr;

        componentsMap.erase(it);
    }

    return false;
}

template <typename CompType>
CompType* EntityManager::GetComponent(EntityID entity) const
{
    int compId = Component::GetID<CompType>();
    assert(compId >= 0);
    assert(compId < NFE_MAX_COMPONENT_TYPES);

    auto& componentsMap = mComponents[compId];
    auto it = componentsMap.find(entity);

    if (it != componentsMap.end())
        return static_cast<CompType*>(it->second);

    return nullptr;
}


template <typename CompType>
void EntityManager::ForEach(std::function<void(EntityID entity, CompType* component)> func)
{
    int compId = Component::GetID<CompType>();
    auto& componentsMap = mComponents[compId];
    for (auto& el : componentsMap)
    {
        EntityID entity = el.first;
        CompType* component = reinterpret_cast<CompType*>(el.second);
        func(entity, component);
    }
}

template <typename CompTypeA, typename CompTypeB>
void EntityManager::ForEach(std::function<void(EntityID entity, CompTypeA* componentA, CompTypeB* componentB)> func)
{
    int compIdA = Component::GetID<CompTypeA>();
    auto& componentsMapA = mComponents[compIdA];
    for (auto& el : componentsMapA)
    {
        EntityID entity = el.first;
        CompTypeA* componentA = reinterpret_cast<CompTypeA*>(el.second);

        int compIdB = Component::GetID<CompTypeB>();
        auto& componentsMapB = mComponents[compIdB];
        auto it = componentsMapB.find(entity);
        if (it != componentsMapB.end())
        {
            CompTypeB* componentB = reinterpret_cast<CompTypeB*>(it->second);
            func(entity, componentA, componentB);
        }
    }
}

} // namespace Scene
} // namespace NFE

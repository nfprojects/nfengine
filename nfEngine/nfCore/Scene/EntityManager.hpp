/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Scene class.
 */

#pragma once

#include "../Core.hpp"
#include "../Components/Component.hpp"

#include <functional>
#include <set>
#include <map>
#include <assert.h>


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
    std::vector<Component*> mInvalidComponents;

public:
    ~EntityManager();

    /**
     * Create an new empty entity.
     * @return Entity ID
     */
    EntityID CreateEntity();

    /**
     * Remove an entity immediately.
     * @param entity Entity ID
     */
    void RemoveEntity(EntityID entity);

    /**
     * Check if an entity with given ID exists.
     * @param entity Entity ID
     * @return True if entity exists
     */
    bool EntityExists(EntityID entity) const;

    /**
     * Add a component to an entity.
     * @param entity    Entity ID
     * @return New component reference
     */
    template <typename CompType>
    CompType* AddComponent(EntityID entity);

    /**
     * Remove a component from an entity immediately.
     * @param entity Entity ID
     * @return True on success
     */
    template <typename CompType>
    bool RemoveComponent(EntityID entity);

    /**
     * Get an entity component.
     * @note The returned pointer is only valid prior to next entity/component addition/removal.
     */
    template <typename CompType>
    CompType* GetComponent(EntityID entity) const;

    /**
     * Iterate through entities that have specific component assigned.
     * @note Used by systems mostly.
     */
    template <typename CompType>
    void ForEach(std::function<void(EntityID entity, CompType* component)> func);

    /**
     * Iterate through entities that have two specific components assigned.
     * @note Used by systems mostly.
     */
    template <typename CompTypeA, typename CompTypeB>
    void ForEach(std::function<void(EntityID entity, CompTypeA* componentA,
                                    CompTypeB* componentB)> func);

    /**
     * Delete all invalid components.
     * @note Should be called only by the engine.
     */
    void FlushInvalidComponents();
};


template <typename CompType>
CompType* EntityManager::AddComponent(EntityID entity)
{
    int compId = CompType::GetID();
    assert(compId >= 0);
    assert(compId < NFE_MAX_COMPONENT_TYPES);

    bool newComponent = true;
    auto& componentsMap = mComponents[compId];
    auto it = componentsMap.find(entity);
    newComponent = (it == componentsMap.end());
    if (!newComponent)
        RemoveComponent<CompType>(entity);

    CompType* ptr = new CompType; // TODO: temporary
    componentsMap[entity] = ptr;

    return ptr;
}

template <typename CompType>
bool EntityManager::RemoveComponent(EntityID entity)
{
    int compId = CompType::GetID();
    assert(compId >= 0);
    assert(compId < NFE_MAX_COMPONENT_TYPES);

    auto& componentsMap = mComponents[compId];
    auto it = componentsMap.find(entity);

    if (it != componentsMap.end())
    {
        CompType* component = static_cast<CompType*>(it->second);
        if (component != nullptr)
        {
            component->Invalidate();
            mInvalidComponents.push_back(component);
        }

        componentsMap.erase(it);
    }

    return false;
}

template <typename CompType>
CompType* EntityManager::GetComponent(EntityID entity) const
{
    int compId = CompType::GetID();
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
    int compId = CompType::GetID();
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
    int compIdA = CompTypeA::GetID();
    auto& componentsMapA = mComponents[compIdA];
    for (auto& el : componentsMapA)
    {
        EntityID entity = el.first;
        CompTypeA* componentA = reinterpret_cast<CompTypeA*>(el.second);

        int compIdB = CompTypeB::GetID();
        auto& componentsMapB = mComponents[compIdB];
        auto it = componentsMapB.find(entity);
        if (it != componentsMapB.end())
        {
            CompTypeB* componentB = reinterpret_cast<CompTypeB*>(it->second);
            func(entity, componentA, componentB);
        }
    }
}

extern CORE_API EntityID gInvalidEntityID;

} // namespace Scene
} // namespace NFE

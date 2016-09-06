/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Scene class.
 */

#pragma once

#include "../Core.hpp"
#include "../Components/Component.hpp"
#include "Entity.hpp"

#include <functional>
#include <set>
#include <map>
#include <assert.h>


namespace NFE {
namespace Scene {


/**
 * Entity listener descriptor.
 */
struct EntityListener
{
    using Callback = std::function<void(EntityID)>;

    uint32 componentTypeMask;
    Callback onCreated;
    Callback onRemoved;
    Callback onChanged;

    EntityListener()
        : componentTypeMask(0)
    { }

    template<typename CompType>
    NFE_INLINE void AddFilter()
    {
        componentTypeMask |= (1 << CompType::GetID());
    }
};

// components table entry
struct ComponentEntry
{
    int32 next;             //< next component in the entity or next free entry in the free list
    int32 type;             //< pointed component type
    Component* component;   //< actual component pointer

    ComponentEntry()
        : next(-1)
        , type(-1)
        , component(nullptr)
    { }
};

/**
 * Entity manager.
 */
class CORE_API EntityManager final
{
    NFE_MAKE_NONCOPYABLE(EntityManager);
    NFE_MAKE_NONMOVEABLE(EntityManager);

    std::vector<Component*> mInvalidComponents;

    std::vector<bool> mEntityBitMap;
    std::vector<int32> mEntityTable;
    EntityID mNextFreeEntity;

    std::vector<ComponentEntry> mComponentsTable;
    int32 mNextFreeComponent;

    // entity listeners matching a component type
    using ListenersType = std::vector<const EntityListener*>;
    ListenersType mListenersPerCompType[NFE_MAX_COMPONENT_TYPES];

    // add component to the table
    bool AddComponentInternal(EntityID entity, Component* component, int componentType);

    // remove component from the table
    bool RemoveComponentInternal(EntityID entity, int componentType);

    // remove component from the table
    Component* GetComponentInternal(EntityID entity, int componentType) const;

public:
    EntityManager();
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
    NFE_INLINE CompType* AddComponent(EntityID entity);

    /**
     * Remove a component from an entity immediately.
     * @param entity Entity ID
     * @return True on success
     */
    template <typename CompType>
    NFE_INLINE bool RemoveComponent(EntityID entity);

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
    void ForEach_DEPRECATED(const std::function<void(EntityID entity, CompType* component)>& func);

    /**
     * Iterate through entities that have two specific components assigned.
     * @note Used by systems mostly.
     */
    template <typename CompTypeA, typename CompTypeB>
    void ForEach_DEPRECATED(const std::function<void(EntityID entity, CompTypeA* componentA,
                                    CompTypeB* componentB)>& func);

    /**
     * Delete all invalid components.
     * @note Should be called only by the engine.
     */
    void FlushInvalidComponents();

    /**
     * Should be called by component, when a property has changed.
     * This will notify all the listeners.
     */
    void OnComponentChanged(EntityID entity, int componentType) const;

    /**
     * Register an entity listener in the manager.
     */
    bool RegisterEntityListener(const EntityListener* listener);

    /**
     * Unregister an entity listener.
     */
    void UnregisterEntityListener(const EntityListener* listener);
};

///////////////////////////////////////////////////////////////////////////////

template <typename CompType>
CompType* EntityManager::AddComponent(EntityID entity)
{
    int typeID = CompType::GetID();
    NFE_ASSERT(typeID >= 0 && typeID < NFE_MAX_COMPONENT_TYPES, "Invalid component type ID");

    // TODO: temporary - use memory pool for each component type
    CompType* component = new CompType;
    if (!AddComponentInternal(entity, component, typeID))
    {
        delete component;
        return nullptr;
    }

    return component;
}

template <typename CompType>
bool EntityManager::RemoveComponent(EntityID entity)
{
    int typeID = CompType::GetID();
    NFE_ASSERT(typeID >= 0 && typeID < NFE_MAX_COMPONENT_TYPES, "Invalid component type ID");
    return RemoveComponentInternal(entity, typeID);
}

template <typename CompType>
CompType* EntityManager::GetComponent(EntityID entity) const
{
    int typeID = CompType::GetID();
    NFE_ASSERT(typeID >= 0 && typeID < NFE_MAX_COMPONENT_TYPES, "Invalid component type ID");
    return static_cast<CompType*>(GetComponentInternal(entity, typeID));
}


// TODO remove
template <typename CompType>
void EntityManager::ForEach_DEPRECATED(const std::function<void(EntityID entity, CompType* component)>& func)
{
    int compId = CompType::GetID();
    for (size_t i = 0; i < mEntityBitMap.size(); ++i)
    {
        if (!mEntityBitMap[i])
            continue;

        int32 next = mEntityTable[i];
        while (next != -1)
        {
            const auto& entry = mComponentsTable[next];
            if (entry.type == compId)
                func(static_cast<EntityID>(i), static_cast<CompType*>(entry.component));

            next = entry.next;
        }
    }
}

// TODO remove
template <typename CompTypeA, typename CompTypeB>
void EntityManager::ForEach_DEPRECATED(const std::function<void(EntityID entity, CompTypeA* componentA, CompTypeB* componentB)>& func)
{
    int compIdA = CompTypeA::GetID();
    int compIdB = CompTypeB::GetID();

    // THIS IS TOTAL BULLSHIT

    for (size_t i = 0; i < mEntityBitMap.size(); ++i)
    {
        if (!mEntityBitMap[i])
            continue;

        CompTypeA* compA = nullptr;
        CompTypeB* compB = nullptr;

        int32 next = mEntityTable[i];
        while (next != -1)
        {
            const auto& entry = mComponentsTable[next];
            if (entry.type == compIdA)
            {
                compA = static_cast<CompTypeA*>(entry.component);
                break;
            }

            next = entry.next;
        }

        next = mEntityTable[i];
        while (next != -1)
        {
            const auto& entry = mComponentsTable[next];
            if (entry.type == compIdB)
            {
                compB = static_cast<CompTypeB*>(entry.component);
                break;
            }

            next = entry.next;
        }

        if (compA && compB)
            func(static_cast<EntityID>(i), compA, compB);
    }
}

extern CORE_API EntityID gInvalidEntityID;

} // namespace Scene
} // namespace NFE

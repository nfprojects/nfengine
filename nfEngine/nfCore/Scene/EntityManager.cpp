/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with
 *         entities managing, events handling, etc.
 */

#include "PCH.hpp"
#include "EntityManager.hpp"
#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Scene {

EntityID gInvalidEntityID = 0;

EntityManager::EntityManager()
    : mNextFreeEntity(-1)
    , mNextFreeComponent(-1)
{
}

EntityManager::~EntityManager()
{
    size_t counter = 0;
    for (size_t i = 0; i < mEntityTable.size(); ++i)
    {
        if (!mEntityBitMap[i])
            continue;

        RemoveEntity(static_cast<EntityID>(i));
        counter++;
    }

    LOG_WARNING("Found %zu not removed entities", counter);

    FlushInvalidComponents();
}

EntityID EntityManager::CreateEntity()
{
    if (mNextFreeEntity == -1) // extend entities table
    {
        size_t oldSize = mEntityBitMap.size();
        size_t newSize = oldSize == 0 ? 32 : (oldSize * 2);
        mEntityBitMap.resize(newSize);
        mEntityTable.resize(newSize);

        // build free list
        for (size_t i = oldSize; i < newSize; ++i)
        {
            mEntityBitMap[i] = false;
            mEntityTable[i] = static_cast<int32>(i + 1);
        }
        mEntityTable[newSize - 1] = static_cast<int32>(-1);
        mNextFreeEntity = static_cast<int32>(oldSize);
    }

    EntityID id = mNextFreeEntity;
    mNextFreeEntity = mEntityTable[mNextFreeEntity];
    mEntityTable[id] = static_cast<int32>(-1); // now this is a head of components list
    mEntityBitMap[id] = true;
    return id;
}

void EntityManager::RemoveEntity(EntityID entity)
{
    // TODO error reporting
    if (!EntityExists(entity))
        return;

    // remove components
    int32 current = mEntityTable[entity];
    while (current != -1)
    {
        auto& entry = mComponentsTable[current];

        // TODO: this will generate duplicated notifications
        for (const auto& listener : mListenersPerCompType[entry.type])
        {
            listener->onRemoved(entity);
        }

        mInvalidComponents.push_back(entry.component);
        entry.component->Invalidate();
        entry.component = nullptr;


        int32 next = entry.next;

        // append to free list
        entry.next = mNextFreeComponent;
        mNextFreeComponent = current;

        current = next;
    }

    // append to free list
    if (mNextFreeEntity != -1)
        mEntityTable[mNextFreeEntity] = entity;
    mNextFreeEntity = entity;

    // remove from bitmap
    mEntityBitMap[entity] = false;
}

bool EntityManager::EntityExists(EntityID entity) const
{
    if (entity >= mEntityBitMap.size())
        return false;

    if (!mEntityBitMap[entity])
        return false;

    return true;
}

void EntityManager::FlushInvalidComponents()
{
    for (auto component : mInvalidComponents)
        delete component;
    mInvalidComponents.clear();
}

bool EntityManager::RegisterEntityListener(const EntityListener* listener)
{
    if (listener->componentTypeMask == 0)
    {
        LOG_ERROR("Must specify at least one component type");
        return false;
    }

    for (uint32 i = 0; i < NFE_MAX_COMPONENT_TYPES; ++i)
    {
        if ((listener->componentTypeMask >> i) & 1)
        {
            mListenersPerCompType[i].push_back(listener);
        }
    }

    return true;
}

void EntityManager::UnregisterEntityListener(const EntityListener* listener)
{
    for (uint32 i = 0; i < NFE_MAX_COMPONENT_TYPES; ++i)
    {
        ListenersType& vector = mListenersPerCompType[i];
        vector.erase(std::remove(vector.begin(), vector.end(), listener), vector.end());
    }
}

void EntityManager::OnComponentChanged(EntityID entity, int componentType) const
{
    NFE_ASSERT(componentType >= 0 && componentType < NFE_MAX_COMPONENT_TYPES, "Invalid component type ID");

    // TODO filtering and buffering

    // notify listeners
    for (const auto& listener : mListenersPerCompType[componentType])
    {
        listener->onChanged(entity);
    }
}

bool EntityManager::AddComponentInternal(EntityID entity, Component* component, int componentType)
{
    if (!EntityExists(entity))
    {
        LOG_ERROR("Entity does not exist");
        return false;
    }

    if (mNextFreeComponent == -1) // extend components table
    {
        size_t oldSize = mComponentsTable.size();
        size_t newSize = oldSize == 0 ? 32 : (oldSize * 2);
        mComponentsTable.resize(newSize);

        // build free list
        for (size_t i = oldSize; i < newSize; ++i)
        {
            mComponentsTable[i].next = static_cast<int32>(i + 1);
        }
        mComponentsTable[newSize - 1].next = static_cast<int32>(-1);
        mNextFreeComponent = static_cast<int32>(oldSize);
    }

    // allocate free component entry
    int32 id = mNextFreeComponent;
    mNextFreeComponent = mComponentsTable[mNextFreeComponent].next;
    mComponentsTable[id].component = component;
    mComponentsTable[id].type = componentType;

    // append to list of entity components
    mComponentsTable[id].next = mEntityTable[entity];
    mEntityTable[entity] = id;

    // notify listeners
    for (const auto& listener : mListenersPerCompType[componentType])
    {
        uint32 collectedMask = 0;
        int32 next = id;
        while (next != -1)
        {
            const auto& entry = mComponentsTable[next];
            collectedMask |= (1 << entry.type);

            if ((listener->componentTypeMask & collectedMask) == listener->componentTypeMask)
            {
                listener->onCreated(entity);
                break;
            }

            next = entry.next;
        }
    }

    component->OnAttachToEntity(this, entity);
    return true;
}

bool EntityManager::RemoveComponentInternal(EntityID entity, int componentType)
{
    if (!EntityExists(entity))
    {
        LOG_ERROR("Entity does not exist");
        return false;
    }

    // notify listeners
    for (const auto& listener : mListenersPerCompType[componentType])
    {
        listener->onRemoved(entity);
    }

    int32 prev = -1;
    int32 current = mEntityTable[entity];
    while (current != -1)
    {
        auto& entry = mComponentsTable[current];
        if (entry.type == componentType)
        {
            mInvalidComponents.push_back(entry.component);
            entry.component->Invalidate();
            entry.component = nullptr;

            // unlink from entity list
            if (prev != -1)
                mComponentsTable[prev].next = entry.next;
            else
                mEntityTable[entity] = entry.next;

            // append to free list
            entry.next = mNextFreeComponent;
            mNextFreeComponent = current;
            return true;
        }

        prev = current;
        current = entry.next;
    }

    // component not found
    return false;
}

Component* EntityManager::GetComponentInternal(EntityID entity, int componentType) const
{
    if (!EntityExists(entity))
    {
        LOG_ERROR("Entity does not exist");
        return nullptr;
    }

    // find component in list
    int32 next = mEntityTable[entity];
    while (next != -1)
    {
        const auto& entry = mComponentsTable[next];
        if (entry.type == componentType)
            return entry.component;

        next = entry.next;
    }

    // component not found
    return nullptr;
}

} // namespace Scene
} // namespace NFE

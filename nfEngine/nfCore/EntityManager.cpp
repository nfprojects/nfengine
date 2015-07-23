/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with
 *         entities managing, events handling, etc.
 */

#include "PCH.hpp"
#include "EntityManager.hpp"

namespace NFE {
namespace Scene {

using namespace Math;

EntityID EntityManager::gLastEntityId = 0;

EntityManager::~EntityManager()
{
    for (int i = 0; i < NFE_MAX_COMPONENT_TYPES; ++i)
    {
        for (auto& el : mComponents[i])
        {
            delete el.second;
        }

        mComponents[i].clear();
    }
}

EntityID EntityManager::CreateEntity()
{
    gLastEntityId++;
    while (EntityExists(gLastEntityId))
        gLastEntityId++;

    mEntities.insert(gLastEntityId);
    return gLastEntityId;
}

bool EntityManager::RemoveEntity(EntityID entity)
{
    auto it = mEntities.find(entity);

    if (it == mEntities.end())
        return false;

    // invalidate components assigned to the entity
    for (int i = 0; i < NFE_MAX_COMPONENT_TYPES; ++i)
    {
        auto it = mComponents[i].find(entity);
        if (it != mComponents[i].end())
        {
            it->second->Invalidate();
            mComponents[i].erase(it);
        }
    }

    mEntities.erase(entity);
    return true;
}

bool EntityManager::EntityExists(EntityID entity) const
{
    return mEntities.find(entity) != mEntities.end();
}

void EntityManager::FlushInvalidComponents()
{
    for (auto component : mInvalidComponents)
        delete component;
    mInvalidComponents.clear();
}

} // namespace Scene
} // namespace NFE

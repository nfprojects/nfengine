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

EntityID gInvalidEntityID = 0;
EntityID EntityManager::gLastEntityId = gInvalidEntityID;

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
    while (EntityExists(gLastEntityId) || gLastEntityId == gInvalidEntityID)
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
        auto comp = mComponents[i].find(entity);
        if (comp != mComponents[i].end())
        {
            comp->second->Invalidate();
            mComponents[i].erase(comp);
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

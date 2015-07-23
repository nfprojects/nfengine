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

    mEntities.erase(entity);
    return true;
}

bool EntityManager::EntityExists(EntityID entity) const
{
    return mEntities.find(entity) != mEntities.end();
}

} // namespace Scene
} // namespace NFE

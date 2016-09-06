/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../Core.hpp"
#include "System.hpp"
#include "../Scene/Entity.hpp"
#include "../Scene/Prefab.hpp"

#include "nfCommon/ThreadPool.hpp"
#include "nfCommon/PackedArray.hpp"

namespace NFE {
namespace Scene {

/**
 * System for managing root entities on the scene.
 */
class EntitySystem
    : public ISystem
{
public:
    EntitySystem(SceneManager* scene);

    /**
     * Update the system.
     */
    void Update(float dt);

    Entity* CreateEntity();

    bool DestroyEntity(Entity* entity);

    // build entity hierarchy from prefab node
    Entity* InstantiatePrefab(const PrefabNode* prefab);

private:
    Common::PackedArray<Entity, uint32, 16> mAllEntities;
};

} // namespace Scene
} // namespace NFE

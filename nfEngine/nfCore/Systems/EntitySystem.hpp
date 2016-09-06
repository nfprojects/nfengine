/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../Core.hpp"
#include "System.hpp"
#include "../Scene/Entity.hpp"

#include "nfCommon/ThreadPool.hpp"
#include "nfCommon/PackedArray.hpp"

namespace NFE {
namespace Scene {

/**
 * System for managing entities on the scene.
 */
class EntitySystem final
    : public ISystem
{
public:
    EntitySystem(SceneManager* scene);

    /**
     * Update the system.
     */
    void Update(float dt) override;

    /**
     * Create a new, empty entity.
     */
    Entity* CreateEntity(Entity* parent);

    /**
     * Mark entity to be deleted (it will be deleted in next frame).
     */
    bool DestroyEntity(Entity* entity);

    void MarkEntityAsDirty(Entity* entity);

private:
    std::vector<Entity> mAllEntities;   // all entities list
    std::vector<uint32> mFreeEntities;  // free entities indicies
};

} // namespace Scene
} // namespace NFE

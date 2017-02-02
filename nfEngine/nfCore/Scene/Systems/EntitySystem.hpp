/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"
#include "../Entity.hpp"

#include <unordered_set>


namespace NFE {
namespace Scene {

/**
 * System for managing entities on the scene.
 */
class CORE_API EntitySystem final
    : public ISystem
{
    NFE_MAKE_NONCOPYABLE(EntitySystem);

public:
    explicit EntitySystem(SceneManager* scene);

    // get all the owned entities
    const std::vector<EntityPtr>& GetAllEntities() const { return mAllEntities; }

    /**
     * Update the system.
     */
    void Update(float dt) override;

    /**
     * Destroy all the entities (immidiate).
     */
    void RemoveAllEntities();

    /**
     * Create a new, empty entity.
     * Do not delete returned pointer, it's managed by the System.
     */
    Entity* CreateEntity(Entity* parent = nullptr);

    /**
     * For internal use only.
     */
    void OnMarkEntityAsDirty(Entity* entity, int flag);

private:
    uint32 mFrameNumber;

    // TODO a better memory allocator is needed here (memory pool)
    // all entities list
    std::vector<EntityPtr> mAllEntities;

    // free entities indicies
    std::vector<EntityID> mFreeEntities;

    // list of entities to be removed
    std::unordered_set<Entity*> mEntitiesToRemove;

    // list of entities that require updating
    std::unordered_set<Entity*> mEntitiesToUpdate;

    void RemovePendingEntities();
    void UpdateEntities();

    // performs entity removal
    // NOTE: for internal use only
    void RemoveEntity(Entity* entity);
};

} // namespace Scene
} // namespace NFE

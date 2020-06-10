/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"
#include "../Entity.hpp"

#include "../../../Common/Containers/DynArray.hpp"
#include "../../../Common/Containers/HashSet.hpp"


namespace NFE {
namespace Scene {

/**
 * System for managing entities on the scene.
 */
class CORE_API EntitySystem final
    : public ISystem
{
    NFE_DECLARE_POLYMORPHIC_CLASS(EntitySystem)
    NFE_MAKE_NONCOPYABLE(EntitySystem)

public:
    static const int ID = 0;

    explicit EntitySystem(Scene& scene);

    // get all the owned entities
    const Common::DynArray<EntityPtr>& GetAllEntities() const { return mAllEntities; }

    /**
     * Update the system.
     */
    void Update(const SystemUpdateContext& context) override;

    /**
     * Destroy all the entities (immediate).
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
    Common::DynArray<EntityPtr> mAllEntities;

    // free entities indicies
    Common::DynArray<EntityID> mFreeEntities;

    // list of entities to be removed
    Common::HashSet<Entity*> mEntitiesToRemove;

    // list of entities that require updating
    Common::HashSet<Entity*> mEntitiesToUpdate;

    void RemovePendingEntities();
    void UpdateEntities();

    // performs entity removal
    // NOTE: for internal use only
    void RemoveEntity(Entity* entity);
};

} // namespace Scene
} // namespace NFE

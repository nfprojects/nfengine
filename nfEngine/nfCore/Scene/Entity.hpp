/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Entity.
 */

#pragma once

#include "../Core.hpp"

#include "nfCommon/Math/Transform.hpp"
#include "nfCommon/Math/Box.hpp"
#include "nfCommon/Memory/Aligned.hpp"


// global-space position was changed
#define NFE_ENTITY_FLAG_MOVED_GLOBAL    (1 << 0)

// local-space position was changed
#define NFE_ENTITY_FLAG_MOVED_LOCAL     (1 << 1)

// the entity is about to be removed
#define NFE_ENTITY_FLAG_TO_REMOVE       (1 << 2)


namespace NFE {
namespace Scene {

/**
 * Scene Entity.
 *
 * Spawned scene node. May be created dynamically or may come from a game object.
 */
class CORE_API NFE_ALIGN16 Entity final : public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(Entity);

public:
    // this will be called only by the Entity System
    Entity(EntityID id, SceneManager* parentScene, Entity* parentEntity = nullptr);
    ~Entity();

    // Getters
    EntityID GetID() const { return mID; }
    size_t GetChildrenNum() const { return mChildren.size(); }
    Entity* GetChild(size_t index) const { return mChildren[index]; }
    Entity* GetParent() const { return mParent; }
    SceneManager* GetScene() const { return mScene; }
    IEntityController* GetController() const { return mController.get(); }

    // Flag checks
    bool HasMovedLocally() const { return (mFlags & NFE_ENTITY_FLAG_MOVED_LOCAL) == NFE_ENTITY_FLAG_MOVED_LOCAL; }
    bool HasMovedGlobally() const { return (mFlags & NFE_ENTITY_FLAG_MOVED_GLOBAL) == NFE_ENTITY_FLAG_MOVED_GLOBAL; }
    bool HasBeenRemoved() const { return (mFlags & NFE_ENTITY_FLAG_TO_REMOVE) == NFE_ENTITY_FLAG_TO_REMOVE; }

    /**
     * Marks the Entity to be removed from the Scene.
     * @param removeChildren    If set to false, the child entities will get attached to the parent of removed entity.
     *                          Otherwise, all child entities will be deleted recursively.
     */
    void Remove(bool removeChildren = true);

    /**
     * Add a component to the entity.
     */
    bool AddComponent(ComponentPtr&& component);

    /**
     * Remove a component from the entity immediately.
     */
    bool RemoveComponent(IComponent* component);
    // TODO remove component by type

    /**
     * Attach a controller.
     * @note    The old controller will be detached and destroyed.
     */
    void AttachController(EntityControllerPtr&& controller);

    /**
     * Remove the controller, if exists.
     * @return  True if the old controller was removed.
     */
    bool DetachController();

    /**
     * Get the entity component by type.
     * @note The returned pointer is only valid prior to next entity/component addition/removal.
     */
    template <typename T>
    T* GetComponent() const;

    /**
     * Attach a new child entity.
     * @return  True on success
     */
    bool AttachChildEntity(Entity* child);

    /**
     * Detach a child entity (make it a scene root entity).
     * @return  True on success
     */
    bool DetachChildEntity(Entity* child);

    /**
     * Set new parent.
     * @return  True on success
     */
    bool SetParentEntity(Entity* newParent);

    /**
     * Get global position and orientation.
     */
    const Math::Transform& GetGlobalTransform() const { return mGlobalTransform; }
    const Math::Vector& GetGlobalPosition() const { return mGlobalTransform.GetTranslation(); }
    const Math::Quaternion& GetGlobalRotation() const { return mGlobalTransform.GetRotations(); }

    /**
     * Get local position and orientation.
     */
    const Math::Transform& GetLocalTransform() const { return mLocalTransform; }
    const Math::Vector& GetLocalPosition() const { return mLocalTransform.GetTranslation(); }
    const Math::Quaternion& GetLocalRotation() const { return mLocalTransform.GetRotations(); }

    /**
     * Set global position and orientation.
     */
    void SetGlobalTransform(const Math::Transform& transform);
    void SetGlobalPosition(const Math::Vector& pos);
    void SetGlobalOrientation(const Math::Quaternion& quat);

    /**
     * Set local position and orientation.
     */
    void SetLocalTransform(const Math::Transform& transform);
    void SetLocalPosition(const Math::Vector& pos);
    void SetLocalOrientation(const Math::Quaternion& orientation);


    /**
     * Calculate axis-aligned bounding box.
     * @param   includeChildren Accumulate children entities boxes?
     * @note    This method is slow, there is no caching. It should be used occasionally.
     */
    Math::Box GetBoundingBox(bool includeChildren = true) const;

    /**
     * Update components.
     * @remarks Should be used by EntitySystem only.
     */
    void Update();

private:
    // Global transform
    Math::Transform mGlobalTransform;

    // Local transform (relative to parent)
    Math::Transform mLocalTransform;

    // parent entity
    Entity* mParent;

    // parent scene
    SceneManager* mScene;

    // attached entity controller
    EntityControllerPtr mController;

    // child entities
    // TODO optimize access (e.g. use pointer to external array)
    std::vector<Entity*> mChildren;

    // attached (and owned) components
    // TODO optimize access (e.g. use pointer to external array)
    std::vector<ComponentPtr> mComponents;

    // unique entity ID (index in the Entity Manager's array of entities)
    EntityID mID;

    // various internal flags (e.g. for marking entity as dirty)
    int mFlags;

    // mark as dirty and notify the Entity System
    void MarkAsDirty(int flags);

    // sync local transform with global transform
    void SyncLocalTransform();

    // sync global transform with local transform
    void SyncGlobalTransform();

    // detach all components (without removing)
    void DetachComponents();
};


template <typename T>
T* Entity::GetComponent() const
{
    // TODO custom RTTI
    for (const ComponentPtr& component : mComponents)
    {
        T* castedComponent = dynamic_cast<T*>(component.get());
        if (castedComponent)
        {
            return castedComponent;
        }
    }

    return nullptr;
}


} // namespace Scene
} // namespace NFE

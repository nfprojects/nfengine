/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Entity.
 */

#pragma once

#include "../Core.hpp"

#include "../../Common/Math/Transform.hpp"
#include "../../Common/Math/Box.hpp"
#include "../../Common/Memory/Aligned.hpp"
#include "../../Common/Containers/DynArray.hpp"
#include "../../Common/Reflection/ReflectionTypeResolver.hpp"


// global-space position was changed
#define NFE_ENTITY_FLAG_MOVED_GLOBAL    (1 << 0)

// local-space position was changed
#define NFE_ENTITY_FLAG_MOVED_LOCAL     (1 << 1)

// the entity is about to be removed
#define NFE_ENTITY_FLAG_TO_REMOVE       (1 << 2)


namespace NFE {
namespace Scene {

/**
 * Scene Entity - a spawned scene node.
 *
 * @note
 * All public methods can be safely called only in two scenarios:
 * 1. From the main thread, outside engine update routine.
 * 2. From owning entity controller's OnEvent method.
 */
class CORE_API NFE_ALIGN(16) Entity final
    : public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(Entity);

public:
    // this will be called only by the Entity System
    explicit Entity(EntityID id, Scene& parentScene, Entity* parentEntity = nullptr);
    ~Entity();

    // Getters
    EntityID GetID() const { return mID; }
    size_t GetChildrenNum() const { return mChildren.Size(); }
    Entity* GetChild(uint32 index) const { return mChildren[index]; }
    Entity* GetParent() const { return mParent; }
    Scene& GetScene() const { return mScene; }

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
    bool RemoveComponent(IComponent* componentToRemove);

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
    bool DetachChildEntity(Entity* childToRemove);

    /**
     * Set new parent.
     * @return  True on success
     */
    bool SetParentEntity(Entity* newParent);

    /**
     * Get global position and orientation.
     */
    const Math::Transform& GetGlobalTransform() const { return mGlobalTransform; }
    const Math::Vector4& GetGlobalPosition() const { return mGlobalTransform.GetTranslation(); }
    const Math::Quaternion& GetGlobalRotation() const { return mGlobalTransform.GetRotation(); }

    /**
     * Get local position and orientation.
     */
    const Math::Transform& GetLocalTransform() const { return mLocalTransform; }
    const Math::Vector4& GetLocalPosition() const { return mLocalTransform.GetTranslation(); }
    const Math::Quaternion& GetLocalRotation() const { return mLocalTransform.GetRotation(); }

    /**
     * Set global position and orientation.
     */
    void SetGlobalTransform(const Math::Transform& transform);
    void SetGlobalPosition(const Math::Vector4& pos);
    void SetGlobalOrientation(const Math::Quaternion& quat);

    /**
     * Set local position and orientation.
     */
    void SetLocalTransform(const Math::Transform& transform);
    void SetLocalPosition(const Math::Vector4& pos);
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
    Scene& mScene;

    // child entities
    Common::DynArray<Entity*> mChildren;

    // attached (and owned) components
    Common::DynArray<ComponentPtr> mComponents;

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
    const RTTI::Type* type = RTTI::GetType<T>();

    for (const ComponentPtr& component : mComponents)
    {
        if (component->GetDynamicType() == type)
        {
            return reinterpret_cast<T*>(component.Get());
        }
    }

    return nullptr;
}


} // namespace Scene
} // namespace NFE

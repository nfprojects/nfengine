/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Entity.
 */

#pragma once

#include "../Core.hpp"

#include "nfCommon/Math/Quaternion.hpp"
#include "nfCommon/Math/Box.hpp"
#include "nfCommon/Aligned.hpp"


// global-space position was changed
#define NFE_ENTITY_FLAG_MOVED_GLOBAL    (1 << 0)

// local-space position was changed
#define NFE_ENTITY_FLAG_MOVED_LOCAL     (1 << 1)

// the entity is about to be removed
#define NFE_ENTITY_FLAG_TO_REMOVE       (1 << 2)


namespace NFE {
namespace Scene {

class Entity;
class SceneManager;
class GameObjectInstance;
class IComponent;

using EntityID = uint32;
using EntityPtr = std::unique_ptr<Entity>;
using ComponentPtr = std::unique_ptr<IComponent>;

/**
 * Scene Entity.
 *
 * Spawned scene node. May be created dynamically or may come from a game object.
 */
NFE_ALIGN16
class CORE_API Entity final
    : public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(Entity);

public:
    // this will be called only by the Entity System
    Entity(EntityID id, SceneManager* parentScene, Entity* parentEntity = nullptr, GameObjectInstance* gameObjectInstance = nullptr);
    ~Entity();

    NFE_INLINE EntityID GetID() const { return mID; }
    NFE_INLINE size_t GetChildrenNum() const { return mChildren.size(); }
    NFE_INLINE Entity* GetChild(size_t index) { return mChildren[index]; }
    NFE_INLINE Entity* GetParent() const { return mParent; }
    NFE_INLINE SceneManager* GetScene() const { return mScene; }

    /**
     * Marks the Entity to be deleted.
     * @param deleteChildren    If set to false, the child entities will get attached to the parent of removed entity.
     *                          Otherwise, all child entities will be deleted recursively.
     */
    void Destroy(bool deleteChildren = true);

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
     * Get the entity component by type.
     * @note The returned pointer is only valid prior to next entity/component addition/removal.
     */
    template <typename T>
    T* GetComponent() const;

    /**
     * Attach a new child entity.
     * @return  True on success
     */
    bool Attach(Entity* child);

    /**
     * Detach a child entity (make it a scene root entity).
     * @return  True on success
     */
    bool Detach(Entity* child);

    /**
     * Set new parent.
     * @return  True on success
     */
    bool SetParent(Entity* newParent);

    /**
     * Get global position and orientation as 4x4 matrix.
     */
    NFE_INLINE Math::Matrix GetGlobalMatrix() const
    {
        return mGlobalMatrix;
    }

    /**
     * Get local position and orientation as 4x4 matrix.
     */
    NFE_INLINE Math::Matrix GetLocalMatrix() const
    {
        return mLocalMatrix;
    }

    /**
     * Get global position in the scene.
     */
    NFE_INLINE Math::Vector GetGlobalPosition() const
    {
        return Math::Vector(mGlobalMatrix.r[3]) & Math::VECTOR_MASK_XYZ;
    }

    /**
     * Get position relative to parent entity.
     */
    NFE_INLINE Math::Vector GetLocalPosition() const
    {
        return Math::Vector(mLocalMatrix.r[3]) & Math::VECTOR_MASK_XYZ;
    }

    /**
     * Get global orientation.
     */
    Math::Matrix GetGlobalOrientation() const;

    /**
     * Get current orientation relative to parent entity.
     */
    Math::Matrix GetLocalOrientation() const;


    /**
     * Set global position and orientation as 4x4 matrix.
     */
    void SetGlobalMatrix(const Math::Matrix& matrix);

    /**
     * Set global position.
     */
    void SetGlobalPosition(const Math::Vector& pos);

    /**
     * Set global orientation using quaternion.
     */
    void SetGlobalOrientation(const Math::Quaternion& quat);

    /**
     * Set global orientation using rotation matrix.
     */
    void SetGlobalOrientation(const Math::Matrix& orientation);


    /**
     * Set position relative to parent entity.
     */
    void SetLocalPosition(const Math::Vector& pos);

    /**
     * Set orientation relative to parent entity.
     */
    void SetLocalOrientation(const Math::Matrix& orientation);

    /**
     * Calculate axis-aligned bounding box.
     * @param   includeChildren Accumulate children entities boxes?
     * @note    This method is slow, there is no caching. It should be used occasionally.
     */
    Math::Box GetBoundingBox(bool includeChildren = true) const;

private:
    // Global transform
    Math::Matrix mGlobalMatrix;

    // Local transform (relative to parent)
    Math::Matrix mLocalMatrix;

    // parent entity
    Entity* mParent;

    // parent scene
    SceneManager* mScene;

    // not null if this Entity is a root of instantiated Game Object
    GameObjectInstance* mGameObjectInstance;

    // child entities
    // TODO optimize access
    std::vector<Entity*> mChildren;

    // attached (and owned) components
    // TODO optimize access
    std::vector<ComponentPtr> mComponents;

    // unique entity ID (index in the Entity Manager's array of entities)
    EntityID mID;

    // various internal flags (e.g. for marking entity as dirty)
    int mFlags;

    // mark as dirty and notify the Entity System
    void MarkAsDirty(int flags);
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

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Entity.
 */

#pragma once

#include "../Core.hpp"
#include "nfCommon/Math/Quaternion.hpp"
#include "nfCommon/Aligned.hpp"

#define NFE_ENTITY_FLAG_MOVED_GLOBAL    (1 << 0)
#define NFE_ENTITY_FLAG_MOVED_LOCAL     (1 << 1)

namespace NFE {
namespace Scene {

class Entity;
class SceneManager;
class GameObjectInstance;
class Component;

using EntityPtr = std::unique_ptr<Entity>;

/**
 * Scene Entity.
 *
 * Spawned scene node. May be created dynamically or may come from a game object.
 */
NFE_ALIGN16
class CORE_API Entity final
    : public Common::Aligned<16>
{
public:
    Entity(SceneManager* parentScene, Entity* parentEntity = nullptr, GameObjectInstance* gameObjectInstance = nullptr);
    ~Entity();

    NFE_INLINE size_t GetChildrenNum() const { return mChildren.size(); }
    NFE_INLINE Entity* GetChild(size_t index) { return mChildren[index]; }
    NFE_INLINE Entity* GetParent() const { return mParent; }
    NFE_INLINE SceneManager* GetScene() const { mParent; }

    /**
     * Add a component to the entity.
     */
    bool AddComponent(std::unique_ptr<Component> component);

    /**
     * Remove a component from the entity immediately.
     */
    bool RemoveComponent(Component* component);

    /**
     * Get the entity component.
     * @note The returned pointer is only valid prior to next entity/component addition/removal.
     */
    template <typename CompType>
    CompType* GetComponent() const;

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

private:
    // TODO maybe represent rotation as quaternion
    // TODO scaling

    // Global transform matrix
    Math::Matrix mGlobalMatrix;

    // Local transform matrix (relative to parent)
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

    // attached components
    std::vector<Component*> mComponents;

    // various internal flags (e.g. for marking entity as dirty)
    int mFlags;

    // mark as dirty and notify entity system
    void MarkAsDirty(int flags);
};


} // namespace Scene
} // namespace NFE

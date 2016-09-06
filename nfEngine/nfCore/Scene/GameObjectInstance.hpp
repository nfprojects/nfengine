/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Entity Controller.
 */

#pragma once

#include "../Core.hpp"
#include "../GameObject/GameObject.hpp"


namespace NFE {
namespace Scene {

class Entity;

/**
 * GameObjectInstance.
 *
 * Instance of a GameObject resource in the scene.
 */
class CORE_API GameObjectInstance final
{
public:
    NFE_INLINE GameObjectInstance(Resource::GameObject* gameObject)
        : mGameObject(gameObject)
        , mEntity(nullptr)
    { }

    // get instanciated root entity
    NFE_INLINE Entity* GetEntity() const { return mEntity; }

    // get instanciated root entity
    NFE_INLINE Resource::GameObject* GetGameObject() const { return mGameObject; }

private:
    Resource::GameObject* mGameObject;  // source Game Object
    Entity* mEntity;                    // spawned root entity
};

} // namespace Scene
} // namespace NFE

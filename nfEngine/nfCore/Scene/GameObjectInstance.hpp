/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Entity Controller.
 */

#pragma once

#include "../Core.hpp"
#include "Events/Event.hpp"
#include "../Resources/GameObject/GameObject.hpp"

#include <unordered_map>


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
    GameObjectInstance(Resource::GameObject* gameObject, Entity* entity);

    // get instantiated root entity
    NFE_INLINE Entity* GetEntity() const { return mEntity; }

    // get instantiated root entity
    NFE_INLINE Resource::GameObject* GetGameObject() const { return mGameObject; }

    // Find entity by game object node name
    Entity* FindEntityByName(const std::string& name) const;

private:
    Resource::GameObject* mGameObject;                  // source Game Object
    Entity* mEntity;                                    // spawned root entity

    std::unordered_map<Resource::GameObjectNode*, Entity*> mEntityMap; // Game Object Node -> Entity
};

using GameObjectInstancePtr = std::unique_ptr<GameObjectInstance>;

} // namespace Scene
} // namespace NFE

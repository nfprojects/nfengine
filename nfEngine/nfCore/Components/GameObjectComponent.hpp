/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../Core.hpp"
#include "Component.hpp"
#include "../GameObject/GameObject.hpp"


namespace NFE {
namespace Scene {

/**
 * Component spawning a Game Object Instance.
 */
NFE_ALIGN16
class CORE_API GameObjectComponent final
    : public ComponentBase<BodyComponent>
{
    NFE_MAKE_NONCOPYABLE(GameObjectComponent)
    NFE_MAKE_NONMOVEABLE(GameObjectComponent)

public:
    GameObjectComponent();
    ~GameObjectComponent();

    NFE_INLINE Resource::GameObject* GetResource() const { return mGameObject; }
    NFE_INLINE GameObjectInstance* GetInstance() const { return mGameObjectInstance; }

    /**
     * Set a new Game Object resource.
     * Spawned instance of the old Game Object will be destroyed.
     */
    bool SetResource(Resource::GameObject* newGameObject);

    /**
     * Spawn Game Object Instance, if not spawned.
     */
    bool Spawn();

    /**
     * Destroy spawned Game Object Instance.
     */
    bool Destroy();

private:
    Resource::GameObject* mGameObject;          // source Game Object resource
    GameObjectInstance* mGameObjectInstance;    // spawned instance of the Game Object
};

} // namespace Scene
} // namespace NFE

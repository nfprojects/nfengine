/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../Core.hpp"


namespace NFE {
namespace Scene {

class GameObjectInstance;
class Event;

/**
 * GameObjectController.
 *
 * Class responsible for controlling a spawned Game Object (Entity).
 * Usually, game classes will inherit from this class to implement game logic.
 */
class CORE_API GameObjectController
{
public:
    NFE_INLINE GameObjectController(GameObjectInstance* instance)
        : mGameObjectInstance(instance)
    { }

    virtual ~GameObjectController() { }

    // get controlled game object instance
    NFE_INLINE GameObjectInstance* GetInstance() const { return mGameObjectInstance; }

    /**
     * Generic event handler.
     */
    virtual void OnEvent(const Event& event);

private:
    GameObjectInstance* mGameObjectInstance;
};

} // namespace Scene
} // namespace NFE

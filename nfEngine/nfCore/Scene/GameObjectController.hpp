/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../Core.hpp"
#include "nfCommon/Math/Quaternion.hpp"


namespace NFE {
namespace Scene {

class GameObjectInstance;

/**
 * GameObjectController.
 *
 * Class responsible for controlling a spawned Game Object.
 * Usually, game classes will inherit from this class to implement game logic.
 */
class CORE_API GameObjectController
{
public:
    NFE_INLINE GameObjectController(GameObjectInstance* entity)
        : mGameObjectInstance(entity)
    { }

    virtual ~GameObjectController() { }

    // get controlled game object instance
    NFE_INLINE GameObjectInstance* GetInstance() const { return mGameObjectInstance; }

    void Update(float dt);

private:
    GameObjectInstance* mGameObjectInstance;
};

} // namespace Scene
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"
#include "../GameObjectInstance.hpp"

#include "nfCommon/ThreadPool.hpp"
#include "nfCommon/PackedArray.hpp"


namespace NFE {
namespace Scene {

/**
 * System for managing Game Object Instances.
 */
class GameObjectSystem
    : public ISystem
{
public:
    GameObjectSystem(SceneManager* scene);

    /**
     * Create an entities hierarchy from given Game Object resource.
     */
    GameObjectInstance* SpawnGameObject(Resource::GameObject* sourceGameObject);

    /**
     * Destroy an existing game object instance.
     */
    void DestroyInstance(GameObjectInstance* instance);

    /**
     * Update the system.
     */
    void Update(float dt);

private:
    std::vector<GameObjectInstancePtr> mInstances;
    std::vector<GameObjectInstancePtr> mInstancesToDestroy;
};

} // namespace Scene
} // namespace NFE

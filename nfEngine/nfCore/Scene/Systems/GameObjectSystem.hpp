/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"
#include "../GameObjectInstance.hpp"


namespace NFE {
namespace Scene {

/**
 * System for managing Game Object Instances.
 */
class CORE_API GameObjectSystem
    : public ISystem
{
    NFE_MAKE_NONCOPYABLE(GameObjectSystem);

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
    void Update(float dt) override;

private:
    std::vector<GameObjectInstancePtr> mInstances;
    std::vector<GameObjectInstancePtr> mInstancesToDestroy;
};

} // namespace Scene
} // namespace NFE

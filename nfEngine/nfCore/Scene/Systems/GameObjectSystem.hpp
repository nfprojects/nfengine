/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"

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
     * Update the system.
     */
    void Update(float dt);

private:
};

} // namespace Scene
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of transform system.
 */

#pragma once

#include "../Core.hpp"
#include "../nfCommon/ThreadPool.hpp"

namespace NFE {
namespace Scene {

class TransformSystem
{
private:
    SceneManager* mScene;

public:
    TransformSystem(SceneManager* scene);

    /**
     * Update system.
     */
    void Update();
};

} // namespace Scene
} // namespace NFE

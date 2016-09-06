/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../Core.hpp"

namespace NFE {
namespace Scene {

class ISystem
{
public:
    NFE_INLINE ISystem(SceneManager* scene)
        : mScene(scene)
    {}

    ~ISystem() { }

protected:
    SceneManager* mScene;
};

} // namespace Scene
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"


namespace NFE {
namespace Scene {

/**
 * Base scene system class.
 */
class CORE_API ISystem
{
public:
    NFE_INLINE ISystem(SceneManager* scene)
        : mScene(scene)
    {}

    virtual ~ISystem() { }

    // get parent scene
    NFE_INLINE SceneManager* GetScene() const { return mScene; }

    // system update method
    virtual void Update(float timeDelta) = 0;

protected:
    SceneManager* mScene;
};

} // namespace Scene
} // namespace NFE

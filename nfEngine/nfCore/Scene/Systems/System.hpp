/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"

#include "nfCommon/Reflection/ReflectionMacros.hpp"


namespace NFE {
namespace Scene {

/**
 * Base scene system class.
 */
class CORE_API ISystem
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ISystem)

public:
    ISystem(SceneManager* scene)
        : mScene(scene)
    {}

    virtual ~ISystem() { }

    // get parent scene
    SceneManager* GetScene() const { return mScene; }

    // system update method
    virtual void Update(float timeDelta) = 0;

private:
    SceneManager* mScene;
};

} // namespace Scene
} // namespace NFE

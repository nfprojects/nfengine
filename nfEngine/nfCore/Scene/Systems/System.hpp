/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"

#include "nfCommon/Reflection/ReflectionClassMacros.hpp"


namespace NFE {
namespace Scene {

struct SystemUpdateContext
{
    uint64 frameNumber;
    double totalTime;
    float timeDelta;

    SystemUpdateContext()
        : frameNumber(0)
        , totalTime(0.0)
        , timeDelta(0.0f)
    { }
};

/**
 * Base scene system class.
 */
class CORE_API ISystem
{
    NFE_DECLARE_POLYMORPHIC_CLASS(ISystem)
    NFE_MAKE_NONCOPYABLE(ISystem)

public:
    ISystem(Scene& scene);
    virtual ~ISystem() { }

    // get parent scene
    Scene& GetScene() const { return mScene; }

    // system update method
    virtual void Update(const SystemUpdateContext& context) = 0;

private:
    Scene& mScene;
};

} // namespace Scene
} // namespace NFE

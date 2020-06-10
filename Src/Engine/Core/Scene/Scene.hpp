/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Scene class.
 */

#pragma once

#include "../Core.hpp"
#include "Systems/System.hpp"

#include "../../Common/Math/Vec4f.hpp"
#include "../../Common/Containers/UniquePtr.hpp"
#include "../../Common/Containers/String.hpp"


namespace NFE {
namespace Scene {

struct SceneUpdateInfo
{
    float timeDelta;
    // TODO system filtering, system parameters, etc.

    SceneUpdateInfo()
        : timeDelta(0.0f)
    { }
};

/**
 * Scene - class responsible for managing and updating scene systems.
 */
class CORE_API Scene final
{
    NFE_MAKE_NONCOPYABLE(Scene)
    NFE_MAKE_NONMOVEABLE(Scene)

public:
    static constexpr int MaxSystems = 8;

    explicit Scene(const Common::StringView& name = "unnamed_scene");
    ~Scene();

    /**
     * Initialize scene systems.
     */
    bool InitializeSystems();

    /**
     * Get the scene name.
     */
    const Common::String& GetName() const { return mName; }

    /**
     * Get system object by type.
     * @remarks This function may return null pointer.
     */
    template<typename SystemType>
    SystemType* GetSystem() const
    {
        static_assert(std::is_base_of<ISystem, SystemType>::value, "Given system type does not derive from ISystem");
        static_assert(SystemType::ID < MaxSystems, "Invalid system");
        return static_cast<SystemType*>(mSystems[typename SystemType::ID].Get());
    }

    void Update(const SceneUpdateInfo& info);

private:
    Common::String mName;

    // this is not very elegant way of storing systems, but it's fast
    Common::UniquePtr<ISystem> mSystems[MaxSystems];

    uint64 mFrameNumber;    // number of frames processed
    double mTotalTime;      // total time elapsed

    void ReleaseSystems();
};


} // namespace Scene
} // namespace NFE

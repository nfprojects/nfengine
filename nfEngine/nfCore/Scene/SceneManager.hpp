/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Scene class.
 */

#pragma once

#include "../Core.hpp"
#include "Systems/System.hpp"

#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Containers/UniquePtr.hpp"

#include <memory>


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
class NFE_ALIGN(16) CORE_API SceneManager final
    : public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(SceneManager);
    NFE_MAKE_NONMOVEABLE(SceneManager);

public:
    static const int MAX_SYSTEMS = 8;

    explicit SceneManager(const std::string& name);
    ~SceneManager();

    /**
     * Initialize scene systems.
     */
    bool InitializeSystems();

    /**
     * Get the scene name.
     */
    const std::string& GetName() const { return mName; }

    /**
     * Get system object by type.
     * @remarks This function may return null pointer.
     */
    template<typename SystemType>
    SystemType* GetSystem() const
    {
        // static_assert(std::is_base_of(ISystem, typename SystemType)::value, "Given system type does not derive from ISystem");
        static_assert(SystemType::ID < MAX_SYSTEMS, "Invalid system");
        return static_cast<SystemType*>(mSystems[typename SystemType::ID].Get());
    }

    /**
     * Launch scene update task. Internal systems will be updated asynchronously.
     *
     * @param deltaTime     Delta time used for physics simulations.
     * @return              Scene update task.
     */
    Common::TaskID BeginUpdate(const SceneUpdateInfo& info);

    /**
     * Launch rendering task.
     */
    // TODO
    // Consider merging the separate "rendering task" with the renderer system update procedure.
    // This will require refactoring the whole Engine::Advance, because the renderer systems will have to
    // know in advance which views they should render to. However, this will make everything more consistent.
    Common::TaskID BeginRendering(const Renderer::View* view);

private:
    std::string mName;

    // this is not very elegant way of storing systems, but it's fast
    Common::UniquePtr<ISystem> mSystems[MAX_SYSTEMS];

    // scene update task
    Common::TaskID mUpdateTask;

    void ReleaseSystems();
};


} // namespace Scene
} // namespace NFE

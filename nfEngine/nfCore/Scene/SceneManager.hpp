/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Scene class.
 */

#pragma once

#include "../Core.hpp"

#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/Memory/Aligned.hpp"


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
// TODO rename to "Scene". But there is name collision with Scene namespace...
class CORE_API NFE_ALIGN16 SceneManager final
    : public Common::Aligned<16>
{
private:
    NFE_MAKE_NONCOPYABLE(SceneManager);

    std::string mName;

    // Systems
    // TODO more clever way of storing the systems (there will be more of them)
    std::unique_ptr<EntitySystem>       mEntitySystem;
    std::unique_ptr<InputSystem>        mInputSystem;
    std::unique_ptr<IPhysicsSystem>     mPhysicsSystem;
    std::unique_ptr<IRendererSystem>    mRendererSystem;

    // scene update task
    Common::TaskID mUpdateTask;

public:
    SceneManager(const std::string& name);
    ~SceneManager();

    NFE_INLINE const std::string& GetName() const { return mName; }

    NFE_INLINE EntitySystem* GetEntitySystem() { return mEntitySystem.get(); }
    NFE_INLINE InputSystem* GetInputSystem() { return mInputSystem.get(); }
    NFE_INLINE IPhysicsSystem* GetPhysicsSystem() { return mPhysicsSystem.get(); }
    NFE_INLINE IRendererSystem* GetRendererSystem() { return mRendererSystem.get(); }

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
};

} // namespace Scene
} // namespace NFE

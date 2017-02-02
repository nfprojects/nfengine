/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Scene class.
 */

#pragma once

#include "../Core.hpp"

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
// TODO rename to "Scene". But there is name collision with Scene namespace...
class NFE_ALIGN(16) CORE_API SceneManager final
    : public Common::Aligned<16>
{
private:
    NFE_MAKE_NONCOPYABLE(SceneManager);
    NFE_MAKE_NONMOVEABLE(SceneManager);

    std::string mName;

    // Systems
    // TODO more clever way of storing the systems (there will be more of them)
    Common::UniquePtr<EntitySystem>       mEntitySystem;
    Common::UniquePtr<InputSystem>        mInputSystem;
    Common::UniquePtr<IPhysicsSystem>     mPhysicsSystem;
    Common::UniquePtr<IRendererSystem>    mRendererSystem;
    Common::UniquePtr<TriggerSystem>      mTriggerSystem;
    Common::UniquePtr<EventSystem>        mEventSystem;

    // scene update task
    Common::TaskID mUpdateTask;

public:
    SceneManager(const std::string& name);
    ~SceneManager();

    const std::string& GetName() const { return mName; }

    // TODO
    EntitySystem* GetEntitySystem() const { return mEntitySystem.Get(); }
    InputSystem* GetInputSystem() const { return mInputSystem.Get(); }
    IPhysicsSystem* GetPhysicsSystem() const { return mPhysicsSystem.Get(); }
    IRendererSystem* GetRendererSystem() const { return mRendererSystem.Get(); }
    TriggerSystem* GetTriggerSystem() const { return mTriggerSystem.Get(); }
    EventSystem* GetEventSystem() const { return mEventSystem.Get(); }

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

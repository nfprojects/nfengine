/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with
 *         entities managing, events handling, etc.
 */

#include "PCH.hpp"
#include "SceneManager.hpp"
#include "Engine.hpp"

// TODO remove this
#include "Renderer/RenderScene.hpp"
#include "Systems/PhysicsSystem.hpp"
#include "Systems/RendererSystem.hpp"
#include "Systems/InputSystem.hpp"
#include "Systems/EntitySystem.hpp"
#include "Systems/EventSystem.hpp"
#include "Systems/TriggerSystem.hpp"

#include "nfCommon/System/Memory.hpp"
#include "nfCommon/System/Memory.hpp"
#include "nfCommon/Utils/ThreadPool.hpp"


namespace NFE {
namespace Scene {

using namespace Math;
using namespace Renderer;
using namespace Resource;

SceneManager::SceneManager(const std::string& name)
    : mName(name)
    , mUpdateTask(NFE_INVALID_TASK_ID)
{ }

SceneManager::~SceneManager()
{
    // Entity system has potentially dependency on every other system (e.g. MeshComponent
    // uses RenderScene), so remove all the pending entities first.
    GetSystem<EntitySystem>()->RemoveAllEntities();
}

bool SceneManager::InitializeSystems()
{
    ReleaseSystems();

    mSystems[EntitySystem::ID] = Common::MakeUniquePtr<EntitySystem>(this);
    mSystems[InputSystem::ID] = Common::MakeUniquePtr<InputSystem>(this);
    mSystems[PhysicsSystem::ID] = Common::MakeUniquePtr<PhysicsSystem>(this);
    mSystems[RendererSystem::ID] = Common::MakeUniquePtr<RendererSystem>(this);
    mSystems[EventSystem::ID] = Common::MakeUniquePtr<EventSystem>(this);
    mSystems[TriggerSystem::ID] = Common::MakeUniquePtr<TriggerSystem>(this);

    return true;
}

void SceneManager::ReleaseSystems()
{
    // TODO release in appropriate order (there may be some inter-system dependencies)
    for (int i = 0; i < MaxSystems; ++i)
    {
        mSystems[i].Reset();
    }
}

Common::TaskID SceneManager::BeginUpdate(const SceneUpdateInfo& info)
{
    // TODO this should be thread pool tasks
    // TODO systems update graph (right now it's very serial...)
    // TODO frame buffering

    for (auto& system : mSystems)
    {
        // ignore uninitialized systems
        if (!system)
            continue;

        system->Update(info.timeDelta);
    }

    // TODO
    return NFE_INVALID_TASK_ID;
}

Common::TaskID SceneManager::BeginRendering(const Renderer::View* view)
{
    RendererSystem* rendererSystem = GetSystem<RendererSystem>();
    if (!rendererSystem)
    {
        // rendering not supported
        return NFE_INVALID_TASK_ID;
    }

    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();
    NFE_ASSERT(threadPool, "Invalid thread pool");

    auto renderCallback = [rendererSystem, view](const Common::TaskContext& context)
    {
        rendererSystem->GetRenderScene()->Render(context, view);
    };

    return threadPool->CreateTask(renderCallback);
}

} // namespace Scene
} // namespace NFE

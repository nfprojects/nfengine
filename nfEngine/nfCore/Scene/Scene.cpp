/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with
 *         entities managing, events handling, etc.
 */

#include "PCH.hpp"
#include "Scene.hpp"

// TODO remove this
#include "Systems/RendererSystem.hpp"
#include "Systems/InputSystem.hpp"
#include "Systems/EntitySystem.hpp"
#include "Systems/EventSystem.hpp"
#include "Systems/TriggerSystem.hpp"


namespace NFE {
namespace Scene {

using namespace Math;
using namespace Resource;

Scene::Scene(const Common::String& name)
    : mName(name)
    , mFrameNumber(0)
    , mTotalTime(0.0)
{ }

Scene::~Scene()
{
    // Entity system has potentially dependency on every other system (e.g. MeshComponent
    // uses RenderScene), so remove all the pending entities first.
    GetSystem<EntitySystem>()->RemoveAllEntities();
}

bool Scene::InitializeSystems()
{
    ReleaseSystems();

    mSystems[EntitySystem::ID] = Common::MakeUniquePtr<EntitySystem>(*this);
    mSystems[InputSystem::ID] = Common::MakeUniquePtr<InputSystem>(*this);
    mSystems[RendererSystem::ID] = Common::MakeUniquePtr<RendererSystem>(*this);
    mSystems[EventSystem::ID] = Common::MakeUniquePtr<EventSystem>(*this);
    mSystems[TriggerSystem::ID] = Common::MakeUniquePtr<TriggerSystem>(*this);

    return true;
}

void Scene::ReleaseSystems()
{
    // TODO release in appropriate order (there may be some inter-system dependencies)
    for (int i = 0; i < MaxSystems; ++i)
    {
        mSystems[i].Reset();
    }
}

void Scene::Update(const SceneUpdateInfo& info)
{
    // TODO this should be thread pool tasks
    // TODO systems update graph (right now it's very serial...)

    SystemUpdateContext updateContext;
    updateContext.timeDelta = info.timeDelta;
    updateContext.totalTime = mTotalTime;
    updateContext.frameNumber = mFrameNumber;

    mFrameNumber++;
    mTotalTime += static_cast<double>(info.timeDelta);

    for (auto& system : mSystems)
    {
        // ignore uninitialized systems
        if (!system)
            continue;

        system->Update(updateContext);
    }
}

} // namespace Scene
} // namespace NFE

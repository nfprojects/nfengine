/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with
 *         entities managing, events handling, etc.
 */

#include "PCH.hpp"
#include "SceneManager.hpp"
#include "Engine.hpp"
#include "nfCommon/Memory.hpp"
#include "Systems/PhysicsSystemImpl.hpp"
#include "Systems/RendererSystem.hpp"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Renderer;
using namespace Resource;

SceneManager::SceneManager()
    : mPhysicsSystem(new PhysicsSystem(this))
    , mRendererSystem(new RendererSystem(this))
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::SetEnvironment(const EnviromentDesc* desc)
{
    if (!Common::MemoryCheck(desc))
        mEnvDesc = *desc;
}

void SceneManager::GetEnvironment(EnviromentDesc* desc) const
{
    if (!Common::MemoryCheck(desc))
        *desc = mEnvDesc;
}

void SceneManager::Update(float deltaTime)
{
    using namespace std::placeholders;
    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    mPhysicsSystem->Update(deltaTime);
    mTransformSystem->Update();
    mEntityManager.FlushInvalidComponents();

    mRendererUpdateTask = threadPool->CreateTask(
        std::bind(&RendererSystem::Update,
                  mRendererSystem.get(), // "this"
                  _1,                    // task context
                  deltaTime));
}

void SceneManager::Render(RenderingData& renderingData)
{
    using namespace std::placeholders;
    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    renderingData.sceneRenderTask =
        threadPool->CreateTask(std::bind(&RendererSystem::Render,
                                         mRendererSystem.get(), // "this"
                                         _1,                    // task context
                                         std::ref(renderingData)),
                               1,                   // instances number
                               NFE_INVALID_TASK_ID, // no parent
                               mRendererUpdateTask);
}

} // namespace Scene
} // namespace NFE
/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with
 *         entities managing, events handling, etc.
 */

#include "PCH.hpp"
#include "SceneManager.hpp"
#include "Engine.hpp"

#include "Systems/PhysicsSystemImpl.hpp"
#include "Systems/RendererSystemImpl.hpp"
#include "Systems/RendererSystem.hpp"
#include "Systems/InputSystem.hpp"
#include "Systems/EntitySystem.hpp"
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
    , mEntitySystem(new EntitySystem(this))
    , mInputSystem(new InputSystem(this))
    , mPhysicsSystem(new PhysicsSystem(this))
    , mRendererSystem(new RendererSystem(this))
    , mUpdateTask(NFE_INVALID_TASK_ID)
{
}

SceneManager::~SceneManager()
{
    mEntitySystem->RemoveAllEntities();
}

Common::TaskID SceneManager::BeginUpdate(const SceneUpdateInfo& info)
{
    // TODO this should be thread pool tasks
    // TODO systems update graph (right now it's very serial...)
    // TODO frame buffering

    mInputSystem->Update(info.timeDelta);
    mPhysicsSystem->Update(info.timeDelta);
    mEntitySystem->Update(info.timeDelta);
    mRendererSystem->Update(info.timeDelta);

    // TODO
    return NFE_INVALID_TASK_ID;
}

Common::TaskID SceneManager::BeginRendering(const Renderer::View* view)
{
    IRendererSystem* rendererSystem = GetRendererSystem();
    if (!rendererSystem)
    {
        // rendering not supported
        return NFE_INVALID_TASK_ID;
    }

    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();
    NFE_ASSERT(threadPool, "Invalid thread pool");

    auto renderCallback = [rendererSystem, view](const Common::TaskContext& context)
    {
        rendererSystem->Render(context, view);
    };

    return threadPool->CreateTask(renderCallback);
}

} // namespace Scene
} // namespace NFE

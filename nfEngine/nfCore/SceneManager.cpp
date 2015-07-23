/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with
 *         entities managing, events handling, etc.
 */

#include "PCH.hpp"
#include "SceneManager.hpp"
#include "Engine.hpp"
#include "../nfCommon/Memory.hpp"
#include "Components/TransformComponent.hpp"
#include "Systems/TransformSystem.hpp"
#include "Systems/PhysicsSystem.hpp"
#include "Systems/RendererSystem.hpp"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Renderer;
using namespace Resource;

SceneManager::SceneManager()
{
    mEventSystem.reset(new EventSystem);
    mTransformSystem.reset(new TransformSystem(this));
    mPhysicsSystem.reset(new PhysicsSystem(this));
    mRendererSystem.reset(new RendererSystem(this));
}

SceneManager::~SceneManager()
{
}

void SceneManager::SetEnvironment(const EnviromentDesc* desc)
{
    if (Common::MemoryCheck(desc) & ACCESS_READ)
        mEnvDesc = *desc;
}

void SceneManager::GetEnvironment(EnviromentDesc* desc) const
{
    if (Common::MemoryCheck(desc) & ACCESS_WRITE)
        *desc = mEnvDesc;
}

void SceneManager::Update(float deltaTime)
{
    if (mEventSystem)
        mEventSystem->Flush();

    if (mPhysicsSystem)
        mPhysicsSystem->Update(deltaTime);

    if (mTransformSystem)
        mTransformSystem->Update();

    if (mRendererSystem)
        mRendererSystem->Update(deltaTime);

    mEntityManager.FlushInvalidComponents();
}

} // namespace Scene
} // namespace NFE
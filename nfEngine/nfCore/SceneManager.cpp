/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with
 *         entities managing, events handling, etc.
 */

#include "PCH.hpp"
#include "SceneManager.hpp"
#include "Engine.hpp"
#include "Systems/PhysicsSystem.hpp"
#include "Systems/RendererSystem.hpp"
#include "../nfCommon/Memory.hpp"
#include "Components/TransformComponent.hpp"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Renderer;
using namespace Resource;

SceneManager::SceneManager()
{
    mEventSystem.reset(new EventSystem);
    mPhysicsSystem.reset(new PhysicsSystem);
    mRendererSystem.reset(new RendererSystem);
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
        mPhysicsSystem->Update(&mEntityManager, deltaTime);

    if (mRendererSystem)
        mRendererSystem->Update(&mEntityManager, deltaTime);

    // TODO: entity manger should update children positions
    mEntityManager.ForEach<TransformComponent>(
        [](EntityID, TransformComponent* transform)
        {
            transform->mFlags = 0;
        });

    mEntityManager.FlushInvalidComponents();
}

} // namespace Scene
} // namespace NFE
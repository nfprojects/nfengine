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
    : mTransformSystem(new TransformSystem(this))
    , mPhysicsSystem(new PhysicsSystem(this))
    , mRendererSystem(new RendererSystem(this))
{
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
    mPhysicsSystem->Update(deltaTime);
    mTransformSystem->Update();
    mRendererSystem->Update(deltaTime);

    mEntityManager.FlushInvalidComponents();
}

} // namespace Scene
} // namespace NFE
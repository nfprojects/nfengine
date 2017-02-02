/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "GameObjectSystem.hpp"
#include "Engine.hpp"
#include "Utils/ConfigVariable.hpp"

namespace NFE {
namespace Scene {

using namespace Math;

GameObjectSystem::GameObjectSystem(SceneManager* scene)
    : ISystem(scene)
{
}

GameObjectInstance* GameObjectSystem::SpawnGameObject(Resource::GameObject* sourceGameObject)
{
    // TODO
    Entity* entity = nullptr;

    GameObjectInstancePtr instance(new GameObjectInstance(sourceGameObject, entity));
    GameObjectInstance* ptr = instance.get();

    // TODO create entities and components

    mInstances.push_back(std::move(instance));
    return ptr;
}

void GameObjectSystem::DestroyInstance(GameObjectInstance* instance)
{
    // TODO
}

void GameObjectSystem::Update(float dt)
{
    // TODO
}

} // namespace Scene
} // namespace NFE

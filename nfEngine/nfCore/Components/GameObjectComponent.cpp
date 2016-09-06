/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "GameObjectComponent.hpp"


namespace NFE {
namespace Scene {

GameObjectComponent::GameObjectComponent()
    : mGameObject(nullptr)
    , mGameObjectInstance(nullptr)
{
}

GameObjectComponent::~GameObjectComponent()
{
    // cleanup
    SetResource(nullptr);
}

bool GameObjectComponent::SetResource(Resource::GameObject* newGameObject)
{
    if (mGameObject == newGameObject)
    {
        // nothing to do
        return true;
    }

    // TODO
    return false;
}

bool GameObjectComponent::Spawn()
{
    // TODO
    return false;
}

bool GameObjectComponent::Destroy()
{
    // TODO
    return false;
}

} // namespace Scene
} // namespace NFE

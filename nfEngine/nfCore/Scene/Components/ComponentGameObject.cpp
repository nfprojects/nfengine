/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "ComponentGameObject.hpp"


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

void GameObjectComponent::OnAttach(Entity* entity)
{
    UNUSED(entity);
}

void GameObjectComponent::OnDetach()
{
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

Math::Box GameObjectComponent::GetBoundingBox() const
{
    // TODO
    return Math::Box();
}


} // namespace Scene
} // namespace NFE

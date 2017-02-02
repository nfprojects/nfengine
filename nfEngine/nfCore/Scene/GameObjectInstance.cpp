/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "GameObjectInstance.hpp"
#include "GameObjectManager.hpp"


namespace NFE {
namespace Scene {

GameObjectInstance::GameObjectInstance(Resource::GameObject* gameObject, Entity* entity)
    : mGameObject(gameObject)
    , mEntity(entity)
{
    // try to create default controller
    mController.reset(GameObjectManager::GetInstance().CreateController(gameObject->GetName()));
}

bool GameObjectInstance::RegisterEventCallback(EventID eventID, const EventCallback& callback)
{
    mEventHandlers[eventID] = callback;
}

} // namespace Scene
} // namespace NFE

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
}

Entity* GameObjectInstance::FindEntityByName(const std::string& name) const
{
    return nullptr;
}

} // namespace Scene
} // namespace NFE

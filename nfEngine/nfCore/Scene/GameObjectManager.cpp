/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "GameObjectManager.hpp"

#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Scene {

GameObjectManager& GameObjectManager::GetInstance()
{
    static GameObjectManager manager;
    return manager;
}

bool GameObjectManager::Register(const std::string& name, const GameObjectControllerFactory& factoryFunc)
{
    if (mFactoryMap.find(name) != mFactoryMap.end())
    {
        LOG_ERROR("GameObject '%s' is already registered", name.c_str());
        return false;
    }

    if (!factoryFunc)
    {
        LOG_ERROR("Invalid factory function for GameObject '%s'", name.c_str());
        return false;
    }

    mFactoryMap[name] = factoryFunc;
    LOG_INFO("GameObject '%s' registered", name.c_str());
    return true;
}

GameObjectController* GameObjectManager::CreateController(const std::string& gameObjectName) const
{
    const auto iter = mFactoryMap.find(gameObjectName);
    if (iter == mFactoryMap.end())
    {
        return nullptr;
    }

    return iter->second();
}

} // namespace Scene
} // namespace NFE

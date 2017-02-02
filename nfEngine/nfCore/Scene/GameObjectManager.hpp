/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../Core.hpp"

#include <functional>

namespace NFE {
namespace Scene {

class GameObject;
class GameObjectController;

/**
 * GameObjectManager.
 *
 * A class for registering game object types and assigning controllers to them.
 *
 * @note This is temporary.
 */
class CORE_API GameObjectManager final
{
public:
    using GameObjectControllerFactory = std::function<GameObjectController*(void)>;

    // get singleton instance
    static GameObjectManager& GetInstance();

    /**
     * Register a factory function for a given Game Object name.
     */
    bool Register(const std::string& name, const GameObjectControllerFactory& factoryFunc);

    /**
     * Create controller for a game object (by name).
     */
    GameObjectController* CreateController(const std::string& gameObjectName) const;

private:
    GameObjectManager();
    ~GameObjectManager();

    std::map<std::string, GameObjectControllerFactory> mFactoryMap;
};


#define NFE_REGISTER_GAME_OBJECT(Name) \
    const bool gGameObjectRegistered_##Name = GameObjectManager::GetInstance().Register(#Name, [](){ return new Name; });


} // namespace Scene
} // namespace NFE

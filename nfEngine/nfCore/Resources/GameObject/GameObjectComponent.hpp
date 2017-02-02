/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"


namespace NFE {
namespace Resource {

class IGameObjectComponent;
using GameObjectComponentPtr = std::unique_ptr<IGameObjectComponent>;

/**
 * Serializable game object component interface.
 */
class CORE_API IGameObjectComponent
{
public:
    virtual ~IGameObjectComponent() { }

    /**
     * Construct actual component from game object data.
     */
    virtual Scene::Component* CreateComponent() = 0;

    // component name, must be unique within the game object
    std::string mName;
};

} // namespace Resource
} // namespace NFE

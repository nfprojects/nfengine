/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "GameObjectComponent.hpp"


namespace NFE {
namespace Resource {

/**
 * Serializable game object component that represents a Game Object Component
 * (component that spawns another game object).
 */
class CORE_API GameObjectComponentMesh : public IGameObjectComponent
{
public:
    // TODO resource ID
    std::string mGameObjectName;

    virtual Scene::Component* CreateComponent() override;
};

} // namespace Resource
} // namespace NFE

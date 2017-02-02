/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "GameObjectComponent.hpp"

#include <string>

namespace NFE {
namespace Resource {

/**
 * Serializable game object component that represents a Body Component.
 */
class CORE_API GameObjectComponentBody : public IGameObjectComponent
{
public:
    virtual Scene::IComponent* CreateComponent() override;

    // TODO resource ID
    std::string mCollisionShapeResourceName;

    float mMass;
};

} // namespace Resource
} // namespace NFE

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
 * Serializable game object component that represents a Mesh Component.
 */
class CORE_API GameObjectComponentMesh : public IGameObjectComponent
{
public:
    std::string meshResourceName;
};

} // namespace Resource
} // namespace NFE

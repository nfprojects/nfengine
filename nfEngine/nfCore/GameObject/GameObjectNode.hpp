/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../Core.hpp"
#include "GameObjectComponent.hpp"


namespace NFE {
namespace Resource {


class GameObjectNode;
using GameObjectNodePtr = std::unique_ptr<GameObjectNode>;

/**
 * Serializable Game Object Node
 */
class CORE_API GameObjectNode
{
public:
    using NodesList = std::vector<GameObjectNodePtr>;
    using ComponentsList = std::vector<GameObjectComponentPtr>;

    NFE_INLINE const NodesList& GetNodes() const { return mChildNodes; }
    NFE_INLINE const ComponentsList& GetComponents() const { return mComponents; }

private:
    // list of child nodes
    NodesList mChildNodes;

    // list of components
    ComponentsList mComponents;
};

} // namespace Resource
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "GameObjectComponent.hpp"

#include "nfCommon/Math/Matrix.hpp"



namespace NFE {
namespace Resource {


class GameObjectNode;
using GameObjectNodePtr = std::unique_ptr<GameObjectNode>;

/**
 * Serializable Game Object Node.
 *
 * Game object node contains collection of Game Object Components and Nodes.
 * After spawning a Game Object, each Node in hierarchy corresponds to a spawned scene Entity.
 */
NFE_ALIGN16
class CORE_API GameObjectNode : public Common::Aligned<16>
{
    // TODO copy constructor must create deep copy!
    // this is not possible without RTTI
    NFE_MAKE_NONCOPYABLE(GameObjectNode);

public:
    using NodesList = std::vector<GameObjectNodePtr>;
    using ComponentsList = std::vector<GameObjectComponentPtr>;

    GameObjectNode();

    NFE_INLINE const Math::Matrix& GetLocalToParentMatrix() const { return mLocalToParent; }
    NFE_INLINE const NodesList& GetNodes() const { return mChildNodes; }
    NFE_INLINE const ComponentsList& GetComponents() const { return mComponents; }

    /**
     * Add a component to the structure.
     */
    bool AddComponent(GameObjectComponentPtr&& component);

    /**
     * Add a node to the structure.
     */
    bool AddNode(GameObjectNodePtr&& node);

    const GameObjectNode* FindNodeByName(const std::string& name) const;

private:
    // relative transform
    Math::Matrix mLocalToParent;

    // node name, must be unique within the game object
    std::string mName;

    // list of child nodes
    NodesList mChildNodes;

    // list of components
    ComponentsList mComponents;
};

} // namespace Resource
} // namespace NFE

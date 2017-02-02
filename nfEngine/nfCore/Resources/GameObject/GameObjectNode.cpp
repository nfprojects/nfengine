/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "GameObjectNode.hpp"

#include "nfCommon/Assertion.hpp"


namespace NFE {
namespace Resource {

// TODO serialization

GameObjectNode::GameObjectNode()
    : mName("node")
{
}

bool GameObjectNode::AddComponent(GameObjectComponentPtr&& component)
{
    mComponents.push_back(std::move(component));
    return true;
}

bool GameObjectNode::AddNode(GameObjectNodePtr&& node)
{
    mChildNodes.push_back(std::move(node));
    return true;
}

const GameObjectNode* GameObjectNode::FindNodeByName(const std::string& name) const
{
    if (mName == name)
    {
        return this;
    }

    for (const auto& child : mChildNodes)
    {
        NFE_ASSERT(child, "Invalid child in game object node");

        const GameObjectNode* result = child->FindNodeByName(name);
        if (result)
        {
            // found
            return result;
        }
    }

    // not found
    return nullptr;
}


} // namespace Resource
} // namespace NFE

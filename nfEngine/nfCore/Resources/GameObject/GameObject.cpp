/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "GameObject.hpp"
#include "GameObjectNode.hpp"


namespace NFE {
namespace Resource {

GameObject::GameObject()
{
}

bool GameObject::OnLoad()
{
    mRoot.reset(new GameObjectNode);

    // TODO parse game object from file

    return true;
}

void GameObject::OnUnload()
{
    // nothing to do here
}

const GameObjectNode* GameObject::FindNodeByName(const std::string& name) const
{
    if (!mRoot)
    {
        return nullptr;
    }

    mRoot->FindNodeByName(name);
}

} // namespace Resource
} // namespace NFE

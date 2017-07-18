/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplateEntity.hpp"

#include "nfCommon/System/Assertion.hpp"


namespace NFE {
namespace Resource {

// TODO serialization

EntityTemplateEntity::EntityTemplateEntity()
    : mName("node")
{
}

bool EntityTemplateEntity::AddComponent(EntityTemplateComponentPtr&& component)
{
    mComponents.PushBack(std::move(component));
    return true;
}

bool EntityTemplateEntity::AddNode(EntityTemplateEntityPtr&& node)
{
    mChildNodes.PushBack(std::move(node));
    return true;
}

const EntityTemplateEntity* EntityTemplateEntity::FindEntityNodeByName(const Common::String& name) const
{
    if (mName == name)
    {
        return this;
    }

    for (const auto& child : mChildNodes)
    {
        NFE_ASSERT(child, "Invalid child in EntityTemplate node");

        const EntityTemplateEntity* result = child->FindEntityNodeByName(name);
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

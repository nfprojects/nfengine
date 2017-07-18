/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplateEntity.hpp"

#include "nfCommon/System/Assertion.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Resource::EntityTemplateEntity)
    NFE_CLASS_MEMBER(mTranslation)
    NFE_CLASS_MEMBER(mRotation)
    NFE_CLASS_MEMBER(mName)
    NFE_CLASS_MEMBER(mChildNodes)
    NFE_CLASS_MEMBER(mComponents)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Resource {


using namespace Common;


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

const EntityTemplateEntity* EntityTemplateEntity::FindEntityNodeByName(const StringView& name) const
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

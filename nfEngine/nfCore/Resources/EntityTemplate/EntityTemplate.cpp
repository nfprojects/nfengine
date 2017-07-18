/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplate.hpp"
#include "EntityTemplateEntity.hpp"


namespace NFE {
namespace Resource {

EntityTemplate::EntityTemplate()
{
}

bool EntityTemplate::OnLoad()
{
    mRoot.reset(new EntityTemplateEntity);

    // TODO parse entity template from file

    return true;
}

void EntityTemplate::OnUnload()
{
    // nothing to do here
}

const EntityTemplateEntity* EntityTemplate::FindNodeByName(const std::string& name) const
{
    if (!mRoot)
    {
        return nullptr;
    }

    return mRoot->FindNodeByName(name);
}

} // namespace Resource
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplate.hpp"
#include "EntityTemplateEntity.hpp"


namespace NFE {
namespace Resource {


using namespace Common;


EntityTemplate::EntityTemplate()
{
}

bool EntityTemplate::OnLoad()
{
    mRoot = Common::MakeUniquePtr<EntityTemplateEntity>();

    // TODO parse entity template from file

    return true;
}

void EntityTemplate::OnUnload()
{
    mRoot.Reset();
}

const EntityTemplateEntity* EntityTemplate::FindEntityNodeByName(const StringView& name) const
{
    if (!mRoot)
    {
        return nullptr;
    }

    return mRoot->FindEntityNodeByName(name);
}

} // namespace Resource
} // namespace NFE

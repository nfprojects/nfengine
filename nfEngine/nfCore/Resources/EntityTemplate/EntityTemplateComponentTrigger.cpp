/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplateComponentTrigger.hpp"
#include "../../Scene/Components/ComponentTrigger.hpp"

#include "nfCommon/Logger/Logger.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Resource::EntityTemplateComponentTrigger)
    NFE_CLASS_PARENT(NFE::Resource::IEntityTemplateComponent)
    NFE_CLASS_MEMBER(mSize)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Resource {

Scene::ComponentPtr EntityTemplateComponentTrigger::CreateComponent() const
{
    Scene::TriggerComponent* component = new Scene::TriggerComponent;
    if (!component)
    {
        LOG_ERROR("Memory alocation failed");
        return nullptr;
    }

    component->SetSize(Math::Vector(mSize));
    return component;
}

} // namespace Resource
} // namespace NFE

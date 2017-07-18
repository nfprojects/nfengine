/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplateComponentController.hpp"
#include "../../Scene/Components/ComponentController.hpp"

#include "nfCommon/Logger/Logger.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Resource::EntityTemplateComponentController)
    NFE_CLASS_PARENT(NFE::Resource::IEntityTemplateComponent)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Resource {

Scene::ComponentPtr EntityTemplateComponentController::CreateComponent() const
{
    Scene::ControllerComponent* component = new Scene::ControllerComponent;
    if (!component)
    {
        LOG_ERROR("Memory alocation failed");
        return nullptr;
    }

    return component;
}

} // namespace Resource
} // namespace NFE

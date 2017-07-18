/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplateComponentController.hpp"
#include "../../Scene/Components/ComponentController.hpp"

#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Resource {

Scene::IComponent* EntityTemplateComponentController::CreateComponent() const
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

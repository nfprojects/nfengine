/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "GameObjectComponentLight.hpp"
#include "../../Scene/Components/ComponentLight.hpp"

#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Resource {

using Scene::LightComponent;


// TODO serialization

GameObjectComponentLight::GameObjectComponentLight()
{

}

Scene::IComponent* GameObjectComponentLight::CreateComponent()
{
    LightComponent* lightComponent = new LightComponent;
    if (!lightComponent)
    {
        LOG_ERROR("Memory alocation failed");
        return nullptr;
    }

    // TODO set light properties
    return lightComponent;
}

} // namespace Resource
} // namespace NFE

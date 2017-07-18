/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplateComponentLight.hpp"
#include "../../Scene/Components/ComponentLight.hpp"

#include "nfCommon/Logger/Logger.hpp"


NFE_BEGIN_DEFINE_CLASS(NFE::Resource::OmniLightDesc)
    NFE_CLASS_MEMBER(radius)
    NFE_CLASS_MEMBER(shadowFadeStart)
    NFE_CLASS_MEMBER(maxShadowDistance)
NFE_END_DEFINE_CLASS()

NFE_BEGIN_DEFINE_CLASS(NFE::Resource::SpotLightDesc)
    NFE_CLASS_MEMBER(nearDist)
    NFE_CLASS_MEMBER(farDist)
    NFE_CLASS_MEMBER(cutoff)
    NFE_CLASS_MEMBER(maxShadowDistance)
NFE_END_DEFINE_CLASS()

NFE_BEGIN_DEFINE_CLASS(NFE::Resource::DirLightDesc)
    NFE_CLASS_MEMBER(farDist)
    NFE_CLASS_MEMBER(lightDist)
    NFE_CLASS_MEMBER(splits)
NFE_END_DEFINE_CLASS()

NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Resource::EntityTemplateComponentLight)
    NFE_CLASS_PARENT(NFE::Resource::IEntityTemplateComponent)
    NFE_CLASS_MEMBER(color)
    NFE_CLASS_MEMBER(omni)
    NFE_CLASS_MEMBER(spot)
    NFE_CLASS_MEMBER(dir)
    NFE_CLASS_MEMBER(shadowMapSize)
    NFE_CLASS_MEMBER(lightType)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Resource {


using Scene::LightComponent;


EntityTemplateComponentLight::EntityTemplateComponentLight()
{

}

Scene::ComponentPtr EntityTemplateComponentLight::CreateComponent() const
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

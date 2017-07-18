/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "EntityTemplateComponentLight.hpp"
#include "../../Scene/Components/ComponentLight.hpp"

#include "nfCommon/Logger/Logger.hpp"


NFE_BEGIN_DEFINE_ENUM(NFE::Resource::LightType)
    NFE_ENUM_OPTION(Unknown)
    NFE_ENUM_OPTION(Omni)
    NFE_ENUM_OPTION(Spot)
    NFE_ENUM_OPTION(Dir)
NFE_END_DEFINE_ENUM()

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

EntityTemplateComponentLight::EntityTemplateComponentLight()
{
}

Scene::ComponentPtr EntityTemplateComponentLight::CreateComponent() const
{
    auto component = Common::MakeUniquePtr<Scene::LightComponent>();
    if (!component)
    {
        LOG_ERROR("Memory alocation failed");
        return nullptr;
    }

    // TODO set light properties
    return component;
}

} // namespace Resource
} // namespace NFE

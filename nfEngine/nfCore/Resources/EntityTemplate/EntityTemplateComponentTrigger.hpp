/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "EntityTemplateComponent.hpp"

#include "nfCommon/Math/Box.hpp"


namespace NFE {
namespace Resource {

/**
 * Serializable EntityTemplate's component that represents a Trigger Component.
 */
class CORE_API EntityTemplateComponentTrigger : public IEntityTemplateComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(EntityTemplateComponentTrigger)

public:
    virtual Scene::IComponent* CreateComponent() const override;

private:
    Math::Box mBox;
};

} // namespace Resource
} // namespace NFE

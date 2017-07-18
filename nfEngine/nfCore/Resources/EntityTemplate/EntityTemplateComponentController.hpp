/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "EntityTemplateComponent.hpp"


namespace NFE {
namespace Resource {


/**
 * Serializable EntityTemplate's component that represents a Controller Component.
 */
class CORE_API EntityTemplateComponentController : public IEntityTemplateComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(EntityTemplateComponentController)

public:
    virtual Scene::IComponent* CreateComponent() const override;

private:
    // TODO script resource reference
};


} // namespace Resource
} // namespace NFE

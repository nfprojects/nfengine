/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "EntityTemplateComponent.hpp"

#include "nfCommon/Math/Float3.hpp"


namespace NFE {
namespace Resource {

/**
 * Serializable EntityTemplate's component that represents a Trigger Component.
 */
class CORE_API EntityTemplateComponentTrigger : public IEntityTemplateComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(EntityTemplateComponentTrigger)

public:
    virtual Scene::ComponentPtr CreateComponent() const override;

private:
    Math::Float3 mSize;
};

} // namespace Resource
} // namespace NFE

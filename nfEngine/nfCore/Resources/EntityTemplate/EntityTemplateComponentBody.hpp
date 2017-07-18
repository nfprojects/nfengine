/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "EntityTemplateComponent.hpp"

#include "nfCommon/Containers/String.hpp"


namespace NFE {
namespace Resource {

/**
 * Serializable EntityTemplate's component that represents a Body Component.
 */
class CORE_API EntityTemplateComponentBody : public IEntityTemplateComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(EntityTemplateComponentBody)

private:
    virtual Scene::IComponent* CreateComponent() const override;

    // TODO resource ref
    Common::String mCollisionShapeResource;

    float mMass;

    // TODO more properties
};

} // namespace Resource
} // namespace NFE

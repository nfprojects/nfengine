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
 * Serializable EntityTemplate's component that represents a Mesh Component.
 */
class CORE_API EntityTemplateComponentMesh : public IEntityTemplateComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(EntityTemplateComponentMesh)

public:
    virtual Scene::ComponentPtr CreateComponent() const override;

private:
    // TODO resource ref
    Common::String mMeshResource;
};

} // namespace Resource
} // namespace NFE

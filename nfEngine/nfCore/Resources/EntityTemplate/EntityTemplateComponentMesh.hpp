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
    virtual Scene::IComponent* CreateComponent() const override;

private:
    // TODO store resource ID here
    std::string mMeshResourceName;
};

} // namespace Resource
} // namespace NFE

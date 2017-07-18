/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "EntityTemplateComponent.hpp"

#include "nfCommon/Reflection/ReflectionMacros.hpp"
#include "nfCommon/Math/Matrix.hpp"
#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/Containers/String.hpp"



namespace NFE {
namespace Resource {


class EntityTemplateEntity;
using EntityTemplateEntityPtr = std::unique_ptr<EntityTemplateEntity>;


/**
 * Serializable EntityTemplate's entity node.
 *
 * EntityTemplate's node contains collection of EntityTemplate's Components and Entities.
 * After spawning an EntityTemplate instance, each Node in hierarchy corresponds to a spawned scene Entity.
 */
class CORE_API NFE_ALIGN(16) EntityTemplateEntity
    : public Common::Aligned<16>
{
    NFE_DECLARE_POLYMORPHIC_CLASS(EntityTemplateEntity)

    // TODO copy constructor must create deep copy!
    NFE_MAKE_NONCOPYABLE(EntityTemplateEntity);

public:
    using NodesList = Common::DynArray<EntityTemplateEntityPtr>;
    using ComponentsList = Common::DynArray<EntityTemplateComponentPtr>;

    EntityTemplateEntity();

    const Math::Matrix& GetLocalToParentMatrix() const { return mLocalToParent; }
    const NodesList& GetNodes() const { return mChildNodes; }
    const ComponentsList& GetComponents() const { return mComponents; }

    /**
     * Add a component to the structure.
     */
    bool AddComponent(EntityTemplateComponentPtr&& component);

    /**
     * Add a node to the structure.
     */
    bool AddNode(EntityTemplateEntityPtr&& node);

    /**
     * Find child entity node by name.
     */
    const EntityTemplateEntity* FindEntityNodeByName(const Common::String& name) const;

private:
    // relative transform
    Math::Matrix mLocalToParent;

    // node name, must be unique within the entity template resource
    Common::String mName;

    // if not empty, describes controller to be used with this entity
    // TODO this should be a script resource in the future
    Common::String mEntityControllerName;

    // list of child nodes
    NodesList mChildNodes;

    // list of components
    ComponentsList mComponents;
};


} // namespace Resource
} // namespace NFE

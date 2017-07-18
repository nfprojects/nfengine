/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "EntityTemplateComponent.hpp"

#include "nfCommon/Reflection/ReflectionClassMacros.hpp"
#include "nfCommon/Math/Quaternion.hpp"
#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/Containers/String.hpp"


namespace NFE {
namespace Resource {


/**
 * Serializable EntityTemplate's entity node.
 *
 * EntityTemplate's node contains collection of EntityTemplate's Components and Entities.
 * After spawning an EntityTemplate instance, each Node in hierarchy corresponds to a spawned scene Entity.
 */
class CORE_API NFE_ALIGN(16) EntityTemplateEntity final
    : public Common::Aligned<16>
{
    NFE_DECLARE_CLASS(EntityTemplateEntity)

public:
    using NodesList = Common::DynArray<EntityTemplateEntityPtr>;
    using ComponentsList = Common::DynArray<EntityTemplateComponentPtr>;

    EntityTemplateEntity();
    EntityTemplateEntity(EntityTemplateEntity&&) = default;
    EntityTemplateEntity& operator = (EntityTemplateEntity&&) = default;

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
    const EntityTemplateEntity* FindEntityNodeByName(const Common::StringView& name) const;

private:
    // relative transform
    Math::Float3 mTranslation;
    Math::Quaternion mRotation;

    // node name, must be unique within the entity template resource
    Common::String mName;

    // list of child nodes (entities)
    NodesList mChildNodes;

    // list of components
    ComponentsList mComponents;
};


} // namespace Resource
} // namespace NFE

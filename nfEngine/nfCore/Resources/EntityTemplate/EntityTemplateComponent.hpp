/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"

#include "nfCommon/Containers/String.hpp"
#include "nfCommon/Reflection/ReflectionMacros.hpp"


namespace NFE {
namespace Resource {


class IEntityTemplateComponent;
using EntityTemplateComponentPtr = std::unique_ptr<IEntityTemplateComponent>;


/**
 * Serializable entity template's component data.
 * Each IEntityTemplateComponent corresponds to a Scene Component when Entity is spawned.
 */
class CORE_API IEntityTemplateComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(IEntityTemplateComponent)

public:
    virtual ~IEntityTemplateComponent() { }

    IEntityTemplateComponent(const IEntityTemplateComponent&) = default;
    IEntityTemplateComponent& operator = (const IEntityTemplateComponent&) = default;
    IEntityTemplateComponent(IEntityTemplateComponent&&) = default;
    IEntityTemplateComponent& operator = (IEntityTemplateComponent&&) = default;

    /**
     * Construct actual scene component from the EntityTemplate's component data.
     */
    virtual Scene::IComponent* CreateComponent() const = 0;

    // component name, must be unique within the entity template resource
    Common::String mName;
};


} // namespace Resource
} // namespace NFE

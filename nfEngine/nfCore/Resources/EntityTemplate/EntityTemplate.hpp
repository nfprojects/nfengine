/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of EntityTemplate resource.
 */

#pragma once

#include "../Resource.hpp"
#include "EntityTemplateEntity.hpp"


namespace NFE {
namespace Resource {

/**
 * EntityTemplate resource
 *
 * Holds scene nodes (entities) structure.
 * Can be spawned on a scene via EntityTemplateSystem.
 */
class CORE_API EntityTemplate final : public ResourceBase
{
    NFE_MAKE_NONCOPYABLE(EntityTemplate);

public:
    EntityTemplate();

    EntityTemplateEntity* GetRootNode() const { return mRoot.Get(); }

    const EntityTemplateEntity* FindEntityNodeByName(const Common::StringView& name) const;

    // ResourceBase interface
    bool OnLoad() override;
    void OnUnload() override;

private:
    EntityTemplateEntityPtr mRoot;
};


} // namespace Resource
} // namespace NFE

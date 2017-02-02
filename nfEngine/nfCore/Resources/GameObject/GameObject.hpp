/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of GameObject resource.
 */

#pragma once

#include "../../Core.hpp"
#include "../Resource.hpp"
#include "GameObjectNode.hpp"

namespace NFE {
namespace Resource {

/**
 * GameObject resource
 *
 * Holds scene nodes (entities) structure.
 * Can be spawned on a scene via GameObjectSystem.
 */
class CORE_API GameObject final : public ResourceBase
{
    NFE_MAKE_NONCOPYABLE(GameObject);

public:
    GameObject();

    NFE_INLINE GameObjectNode* GetRootNode() const { return mRoot.get(); }

    const GameObjectNode* FindNodeByName(const std::string& name) const;

    // ResourceBase interface
    bool OnLoad() override;
    void OnUnload() override;

private:
    GameObjectNodePtr mRoot;
};


} // namespace Resource
} // namespace NFE

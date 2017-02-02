/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of GameObject resource.
 */

#pragma once

#include "../../Core.hpp"
#include "../Resource.hpp"


namespace NFE {
namespace Resource {

class GameObjectNode;
using GameObjectNodePtr = std::unique_ptr<GameObjectNode>;

/**
 * GameObject resource
 *
 * Holds scene nodes (entities) structure.
 */
class CORE_API GameObject : public ResourceBase
{
    NFE_MAKE_NONCOPYABLE(GameObject);

public:
    NFE_INLINE GameObjectNode* GetRootNode() const { return mRoot.get(); }

    // ResourceBase interface
    bool OnLoad() override;
    void OnUnload() override;

private:
    GameObjectNodePtr mRoot;
};


} // namespace Resource
} // namespace NFE

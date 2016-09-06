/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Prefab.
 */

#pragma once

#include "../Core.hpp"
#include "../Resources/Resource.hpp"


namespace NFE {
namespace Resource {

class GameObjectNode;
using GameObjectNodePtr = std::unique_ptr<GameObjectNode>;

// GameObject resource
// Holds scene nodes (entities) structure
class CORE_API GameObject : public ResourceBase
{
public:
    NFE_INLINE GameObjectNodePtr* GetRootNode() const { return mRoot.get(); }

private:
    std::unique_ptr<GameObjectNodePtr> mRoot;
};


} // namespace Resource
} // namespace NFE

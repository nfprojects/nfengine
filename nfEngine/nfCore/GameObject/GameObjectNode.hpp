/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Prefab.
 */

#pragma once

#include "../Core.hpp"
#include "nfCommon/Math/Quaternion.hpp"


namespace NFE {
namespace Scene {

class CORE_API PrefabNode
{
public:
    std::string name;
};

class CORE_API PrefabComponent
{
public:
    // TODO: packed component data
};

// prefab node - entity
class CORE_API PrefabNodeEntity : public PrefabNode
{
public:
};

// prefab node - prefab
class CORE_API PrefabNodePrefab : public PrefabNode
{
public:
};

} // namespace Scene
} // namespace NFE

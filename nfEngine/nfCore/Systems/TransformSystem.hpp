/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of transform system.
 */

#pragma once

#include "../Core.hpp"
#include "../Scene/EntityManager.hpp"

namespace NFE {
namespace Scene {

class CORE_API TransformSystem
{
private:
    EntityManager* mEntityManager;

    void UpdateComponent(TransformComponent* component);

public:
    TransformSystem(SceneManager* scene);

    /**
     * Change entity's parent.
     */
    bool SetParent(EntityID entity, EntityID newParent);

    /**
     * Update system.
     */
    void Update();
};

} // namespace Scene
} // namespace NFE

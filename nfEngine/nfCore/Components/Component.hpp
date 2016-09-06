/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base class declarations.
 */

#pragma once

#include "../Core.hpp"
#include "nfCommon/nfCommon.hpp"

#include <vector>


namespace NFE {
namespace Scene {

class Entity;
class SceneManager;

/**
 * Base class for all component types.
 */
class CORE_API Component
{
public:
    Component();
    virtual ~Component();

    /**
     * Get parent Entity pointer.
     */
    NFE_INLINE Entity* GetEntity() const { return mEntity; }

    /**
     * Get parent Scene.
     */
    SceneManager* GetScene() const;

protected:
    /**
     * Called when attached to an entity.
     */
    virtual void OnAttach(Entity* entity);

    /**
     * Called when detached from the entity.
     */
    virtual void OnDetach();

private:
    Entity* mEntity;    // parent entity
};

} // namespace Scene
} // namespace NFE

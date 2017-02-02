/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base class declarations.
 */

#pragma once

#include "../../Core.hpp"

#include "nfCommon/Math/Box.hpp"


namespace NFE {
namespace Scene {

/**
 * Base class for all component types.
 */
class CORE_API IComponent
{
    friend class Entity;

public:
    IComponent();
    virtual ~IComponent();

    /**
     * Get parent Entity pointer.
     */
    NFE_INLINE Entity* GetEntity() const { return mEntity; }

    /**
     * Get parent Scene.
     */
    SceneManager* GetScene() const;

    /**
     * Get bounding box in entity space. May return empty box.
     */
    virtual Math::Box GetBoundingBox() const = 0;

    /**
     * Called when attached to an entity.
     */
    virtual void OnAttach();

    /**
     * Called when detached from the entity.
     */
    virtual void OnDetach();

    /**
     * Called when parent entity was moved.
     */
    virtual void OnUpdate();

private:
    Entity* mEntity;    // parent entity
};

} // namespace Scene
} // namespace NFE

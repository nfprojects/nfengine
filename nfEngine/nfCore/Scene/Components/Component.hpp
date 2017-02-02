/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base class declarations.
 */

#pragma once

#include "../../Core.hpp"

#include "nfCommon/Reflection/ReflectionMacros.hpp"
#include "nfCommon/Math/Box.hpp"


namespace NFE {
namespace Scene {

/**
 * Base class for all component types.
 */
class CORE_API IComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(IComponent)

public:
    IComponent();
    virtual ~IComponent();

    IComponent(const IComponent&) = default;
    IComponent(IComponent&&) = default;
    IComponent& operator=(const IComponent&) = default;
    IComponent& operator=(IComponent&&) = default;

    /**
     * Get parent Entity pointer.
     */
    Entity* GetEntity() const { return mEntity; }

    /**
     * Get parent Scene.
     */
    SceneManager* GetScene() const;

    /**
     * Get bounding box in entity space. May return empty box.
     */
    virtual Math::Box GetBoundingBox() const;

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

    /**
     * Attach/detach to an entity.
     * @note    Should be called only by Entity class.
     */
    void Attach(Entity& entity);
    void Detach(Entity& entity);

private:
    // parent entity
    Entity* mEntity;

};

} // namespace Scene
} // namespace NFE

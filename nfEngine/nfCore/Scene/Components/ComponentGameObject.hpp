/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "Component.hpp"
#include "../../Resources/GameObject/GameObject.hpp"
#include "../GameObjectInstance.hpp"


namespace NFE {
namespace Scene {

/**
 * Component that spawns another Game Object Instance.
 */
class CORE_API GameObjectComponent final
    : public IComponent
{
public:
    GameObjectComponent();
    ~GameObjectComponent();

    NFE_INLINE Resource::GameObject* GetResource() const { return mGameObject; }
    NFE_INLINE GameObjectInstance* GetInstance() const { return mGameObjectInstance; }

    // IComponent interface implementation
    virtual Math::Box GetBoundingBox() const override;
    virtual void OnAttach() override;
    virtual void OnDetach() override;

    /**
     * Set a new Game Object resource.
     * Spawned instance of the old Game Object will be destroyed.
     */
    bool SetResource(Resource::GameObject* newGameObject);

    // TODO OnAttach / OnDetach

    /**
     * Spawn Game Object Instance, if not spawned.
     */
    bool Spawn();

    /**
     * Destroy spawned Game Object Instance.
     */
    bool Destroy();

private:
    Resource::GameObject* mGameObject;          // source Game Object resource
    GameObjectInstance* mGameObjectInstance;    // spawned instance of the Game Object
};

} // namespace Scene
} // namespace NFE

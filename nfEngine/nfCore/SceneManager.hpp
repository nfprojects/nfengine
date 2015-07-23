/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Scene class.
 */

#pragma once

#include "Core.hpp"
#include "../nfCommon/ThreadPool.hpp"
#include "SceneEvent.hpp"
#include "EntityManager.hpp"

namespace NFE {
namespace Scene {

NFE_ALIGNED_CLASS(class CORE_API EnviromentDesc)
{
public:
    Math::Vector ambientLight;
    Math::Vector backgroundColor;

    EnviromentDesc()
    {
        ambientLight = Math::Vector(0.3f, 0.3f, 0.3f);
        backgroundColor = Math::Vector(0.3f, 0.3f, 0.3f);
    }
};

/**
 * Scene manager.
 */
NFE_ALIGNED_CLASS(class CORE_API SceneManager)
{
private:
    EntityManager mEntityManager;

    // environment
    EnviromentDesc mEnvDesc;

    /// Systems
    std::unique_ptr<EventSystem> mEventSystem;
    std::unique_ptr<PhysicsSystem> mPhysicsSystem;
    std::unique_ptr<RendererSystem> mRendererSystem;

public:
    SceneManager();
    ~SceneManager();

    void SetEnvironment(const EnviromentDesc* desc);
    void GetEnvironment(EnviromentDesc* desc) const;

    /**
     * Calculate physics, prepare scene for rendering.
     * @param deltaTime Delta time used for physics simulations.
     */
    void Update(float deltaTime);

    NFE_INLINE EntityManager* GetEntityManager()
    {
        return &mEntityManager;
    }

    NFE_INLINE RendererSystem* GetRendererSystem()
    {
        return mRendererSystem.get();
    }

    NFE_INLINE EventSystem* GetEventSystem()
    {
        return mEventSystem.get();
    }
};

} // namespace Scene
} // namespace NFE

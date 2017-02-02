/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Scene class.
 */

#pragma once

#include "../Core.hpp"

#include "nfCommon/ThreadPool.hpp"
#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/Aligned.hpp"


namespace NFE {
namespace Scene {

class RenderingData;

NFE_ALIGN16
class CORE_API EnviromentDesc : public Common::Aligned<16>
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
 * Scene.
 *
 * Class responsible for holding and ticking scene systems.
 */
NFE_ALIGN16
class CORE_API SceneManager final : public Common::Aligned<16>
{
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

    /**
     * Launch rendering task.
     * @param renderingData Temporary rendering data.
     */
    void Render(RenderingData& renderingData);

    NFE_INLINE EntitySystem* GetEntitySystem() { return mEntitySystem.get(); }
    NFE_INLINE GameObjectSystem* GetGameObjectSystem() { return mGameObjectSystem.get(); }
    NFE_INLINE InputSystem* GetInputSystem() { return mInputSystem.get(); }
    NFE_INLINE PhysicsSystem* GetPhysicsSystem() { return mPhysicsSystem.get(); }
    NFE_INLINE RendererSystem* GetRendererSystem() { return mRendererSystem.get(); }

private:
    // environment
    EnviromentDesc mEnvDesc;

    Common::TaskID mRendererUpdateTask;

    // Systems
    // TODO more clever way of storing the systems
    std::unique_ptr<EntitySystem>       mEntitySystem;
    std::unique_ptr<GameObjectSystem>   mGameObjectSystem;
    std::unique_ptr<InputSystem>        mInputSystem;
    std::unique_ptr<PhysicsSystem>      mPhysicsSystem;
    std::unique_ptr<RendererSystem>     mRendererSystem;

    Common::TaskID mUpdateTask;
};

} // namespace Scene
} // namespace NFE

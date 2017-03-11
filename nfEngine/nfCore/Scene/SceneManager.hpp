/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Scene class.
 */

#pragma once

#include "../Core.hpp"
#include "EntityManager.hpp"

#include "nfCommon/Utils/ThreadPool.hpp"
#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/Memory/Aligned.hpp"


namespace NFE {
namespace Scene {

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
 * Scene manager.
 */
NFE_ALIGN16
class CORE_API SceneManager : public Common::Aligned<16>
{
private:
    EntityManager mEntityManager;

    // environment
    EnviromentDesc mEnvDesc;

    Common::TaskID mRendererUpdateTask;

    /// Systems
    std::unique_ptr<TransformSystem> mTransformSystem;
    std::unique_ptr<PhysicsSystem> mPhysicsSystem;
    std::unique_ptr<RendererSystem> mRendererSystem;

    Common::TaskID mUpdateTask;

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

    NFE_INLINE EntityManager* GetEntityManager()
    {
        return &mEntityManager;
    }

    NFE_INLINE TransformSystem* GetTransformSystem()
    {
        return mTransformSystem.get();
    }

    NFE_INLINE RendererSystem* GetRendererSystem()
    {
        return mRendererSystem.get();
    }
};

} // namespace Scene
} // namespace NFE

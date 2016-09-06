/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of physics system.
 */

#pragma once

#include "../Core.hpp"
#include "Scene/EntityManager.hpp"
#include "nfCommon/Aligned.hpp"
#include "nfCommon/ThreadPool.hpp"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


namespace NFE {
namespace Scene {

struct BodyEntry
{

};

NFE_ALIGN16
class PhysicsSystem : public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(PhysicsSystem);
    NFE_MAKE_NONMOVEABLE(PhysicsSystem);

    // parent scene
    SceneManager* mScene;

    // entity listeners for body entities
    EntityListener mBodiesListener;

    btEmptyShape mEmptyCollisionShape;

    /// Bullet Physics objects
    std::unique_ptr<btBroadphaseInterface> mBroadphase;
    std::unique_ptr<btDefaultCollisionConfiguration> mCollsionConfig;
    std::unique_ptr<btCollisionDispatcher> mDispatcher;
    std::unique_ptr<btSequentialImpulseConstraintSolver> mSolver;
    std::unique_ptr<btDiscreteDynamicsWorld> mDynamicsWorld;

    void UpdatePhysics(float dt);
    void ProcessContacts();

    void OnBodyEntityCreated(EntityID entity);
    void OnBodyEntityRemoved(EntityID entity);
    void OnBodyEntityChanged(EntityID entity);

public:
    PhysicsSystem(SceneManager* scene);
    ~PhysicsSystem();

    /**
     * Update the system.
     */
    void Update(float dt);
};

} // namespace Scene
} // namespace NFE

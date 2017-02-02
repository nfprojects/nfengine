/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of physics system.
 */

#pragma once

#include "../../Core.hpp"
#include "PhysicsSystem.hpp"

#include "nfCommon/Containers/PackedArray.hpp"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


namespace NFE {
namespace Scene {

/**
 * Physics Body proxy managed by the physics system.
 */
class NFE_ALIGN(16) PhysicsBodyProxy
    : public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(PhysicsBodyProxy)

public:
    PhysicsBodyProxyInfo info;
    std::unique_ptr<btDefaultMotionState> motionState;
    std::unique_ptr<btRigidBody> rigidBody;

    PhysicsBodyProxy() = default;
    PhysicsBodyProxy(PhysicsBodyProxy&& rhs) = default;
    PhysicsBodyProxy& operator=(PhysicsBodyProxy&& rhs) = default;
};

/**
 * Scene system responsible for simulating physics.
 */
class NFE_ALIGN(16) PhysicsSystem
    : public IPhysicsSystem
    , public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(PhysicsSystem)

private:
    friend void PhysicsUpdateCallback(void* userData, int instance, int threadID);

    SceneManager* mScene;

    btEmptyShape mEmptyCollisionShape;

    /// Bullet Physics objects
    std::unique_ptr<btBroadphaseInterface> mBroadphase;
    std::unique_ptr<btDefaultCollisionConfiguration> mCollsionConfig;
    std::unique_ptr<btCollisionDispatcher> mDispatcher;
    std::unique_ptr<btSequentialImpulseConstraintSolver> mSolver;
    std::unique_ptr<btDiscreteDynamicsWorld> mDynamicsWorld;

    // list of all existing body proxies in this system
    Common::PackedArray<PhysicsBodyProxy, PhysicsProxyID, 16> mBodyProxies;

public:
    PhysicsSystem(SceneManager* scene);
    ~PhysicsSystem();

    /**
     * Update the system.
     */
    void Update(float timeDelta) override;

    // IPhysicsSystem interface implementation
    PhysicsProxyID CreateBodyProxy(const PhysicsBodyProxyInfo& info) override;
    bool UpdateBodyProxy(const PhysicsProxyID proxyID, const PhysicsBodyProxyInfo& newInfo) override;
    void DeleteBodyProxy(const PhysicsProxyID proxyID) override;
};

} // namespace Scene
} // namespace NFE

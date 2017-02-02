/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of physics scene.
 */

#pragma once

#include "../Core.hpp"
#include "PhysicsScene.hpp"

#include "nfCommon/Containers/UniquePtr.hpp"
#include "nfCommon/Containers/PackedArray.hpp"
#include "nfCommon/Memory/Aligned.hpp"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


namespace NFE {
namespace Physics {


/**
 * Physics Body proxy managed by the physics scene.
 */
class NFE_ALIGN(16) BodyProxy
    : public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(BodyProxy)

public:
    BodyProxyInfo info;

    // TODO store objects here instead of pointers
    Common::UniquePtr<btDefaultMotionState> motionState;
    Common::UniquePtr<btRigidBody> rigidBody;

    BodyProxy() = default;
    ~BodyProxy();
    BodyProxy(BodyProxy&& rhs) = default;
    BodyProxy& operator=(BodyProxy&& rhs) = default;
};


/**
 * Physics scene - allows for placing and simulating physics proxies (objects).
 */
class NFE_ALIGN(16) PhysicsScene
    : public IPhysicsScene
    , public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(PhysicsScene)

private:
    friend void PhysicsUpdateCallback(void* userData, int instance, int threadID);

    btEmptyShape mEmptyCollisionShape;

    /// Bullet Physics objects
    Common::UniquePtr<btBroadphaseInterface> mBroadphase;
    Common::UniquePtr<btDefaultCollisionConfiguration> mCollsionConfig;
    Common::UniquePtr<btCollisionDispatcher> mDispatcher;
    Common::UniquePtr<btSequentialImpulseConstraintSolver> mSolver;
    Common::UniquePtr<btDiscreteDynamicsWorld> mDynamicsWorld;

    // list of all existing body proxies in this system
    Common::PackedArray<BodyProxy, ProxyID, 16> mBodyProxies;

public:
    PhysicsScene();
    ~PhysicsScene();

    // IPhysicsScene interface implementation
    void Update(float timeDelta) override;
    ProxyID CreateBodyProxy(const BodyProxyInfo& info) override;
    bool UpdateBodyProxy(const ProxyID proxyID, const BodyProxyInfo& newInfo) override;
    void DeleteBodyProxy(const ProxyID proxyID) override;
};


} // namespace Physics
} // namespace NFE

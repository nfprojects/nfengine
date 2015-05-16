/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of physics manager.
 */

#pragma once

#include "Core.hpp"
#include "Aligned.hpp"
#include "../nfCommon/ThreadPool.hpp"

namespace NFE {
namespace Scene {

NFE_ALIGN16
class PhysicsWorld : public Util::Aligned
{
    friend class SceneManager;
    friend class BodyComponent;
    friend void PhysicsUpdateCallback(void* pUserData, int Instance, int ThreadID);

private:
    SceneManager* mScene;

    Common::TaskID mPhysicsTask;
    volatile bool mRunning;

    // Bullet Physics objects
    btBroadphaseInterface* mBroadphase;
    btDefaultCollisionConfiguration* mCollsionConfig;
    btCollisionDispatcher* mDispatcher;
    btSequentialImpulseConstraintSolver* mSolver;
    btDiscreteDynamicsWorld* mDynamicsWorld;

    float mDeltaTime;
    void UpdatePhysics();

public:
    PhysicsWorld(SceneManager* pScene);
    ~PhysicsWorld();

    //Start physics update task
    void StartUpdate(float deltaTime);

    //Wait for physics update task
    void Wait();

    // process all collisions
    void ProcessContacts();
};

} // namespace Scene
} // namespace NFE

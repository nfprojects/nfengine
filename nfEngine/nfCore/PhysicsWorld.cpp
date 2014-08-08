/**
    NFEngine project

    \file   PhysicsWorld.cpp
    \brief  Definitions of physics manager.
*/

#include "stdafx.h"
#include "Globals.h"
#include "PhysicsWorld.h"
#include "Engine.h"
#include "SceneEvent.h"
#include "SceneManager.h"
#include "Performance.h"
#include "../nfCommon/Timer.h"

namespace NFE {
namespace Scene {

void PhysicsUpdateCallback(void* pUserData, int Instance, int ThreadID)
{
    PhysicsWorld* pWorld = (PhysicsWorld*)pUserData;

    Common::Timer timer;
    timer.Start();
    {
        pWorld->mDynamicsWorld->stepSimulation(pWorld->mDeltaTime, 40, 1.0f / 90.0f);
    }
    Util::g_FrameStats.physics = timer.Stop();
}

PhysicsWorld::PhysicsWorld(SceneManager* pScene)
{
    mScene = pScene;

    mPhysicsTask = 0;
    mRunning = false;

    mBroadphase = new btDbvtBroadphase();
    mCollsionConfig = new btDefaultCollisionConfiguration();
    mDispatcher = new btCollisionDispatcher(mCollsionConfig);
    mSolver = new btSequentialImpulseConstraintSolver;
    mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver,
            mCollsionConfig);

    //world->getSimulationIslandManager()->setSplitIslands(false);
    mDynamicsWorld->getDispatchInfo().m_useContinuous = true;

    mDynamicsWorld->getSolverInfo().m_solverMode |= SOLVER_SIMD | SOLVER_USE_WARMSTARTING;
    mDynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));
    //mDynamicsWorld->setGravity(btVector3(0.0f, 0.0f, 0.0f));

    /*
    // MULTITHREADED PHYSICS = CRASH

    btDefaultCollisionConstructionInfo cci;
    cci.m_defaultMaxPersistentManifoldPoolSize = 32768;
    mCollsionConfig = new btDefaultCollisionConfiguration(cci);

    int taskThreads = 4;
    m_ThreadSupportCollision = new Win32ThreadSupport(Win32ThreadSupport::Win32ThreadConstructionInfo(
                                "collision",
                                processCollisionTask,
                                createCollisionLocalStoreMemory,
                                taskThreads));

    mDispatcher = new SpuGatheringCollisionDispatcher(m_ThreadSupportCollision, taskThreads, mCollsionConfig);
    //mBroadphase = new btDbvtBroadphase();

    btVector3 worldAabbMin(-1000,-1000,-1000);
    btVector3 worldAabbMax(1000,1000,1000);
    mBroadphase = new btAxisSweep3(worldAabbMin, worldAabbMax, 32766);

    Win32ThreadSupport::Win32ThreadConstructionInfo threadConstructionInfo("solverThreads", SolverThreadFunc, SolverlsMemoryFunc, taskThreads);
    m_ThreadSupportSolver = new Win32ThreadSupport(threadConstructionInfo);
    m_ThreadSupportSolver->startSPU();

    mSolver = new btParallelConstraintSolver(m_ThreadSupportSolver);

    //this solver requires the contacts to be in a contiguous pool, so avoid dynamic allocation
    mDispatcher->setDispatcherFlags(btCollisionDispatcher::CD_DISABLE_CONTACTPOOL_DYNAMIC_ALLOCATION);


    btDiscreteDynamicsWorld* world = new btDiscreteDynamicsWorld(mDispatcher,mBroadphase,mSolver,mCollsionConfig);
    mDynamicsWorld = world;

    world->getSimulationIslandManager()->setSplitIslands(false);
    mDynamicsWorld->getSolverInfo().m_numIterations = 4;
    mDynamicsWorld->getSolverInfo().m_solverMode = SOLVER_SIMD+SOLVER_USE_WARMSTARTING;

    mDynamicsWorld->getDispatchInfo().m_useContinuous = true;
    mDynamicsWorld->getDispatchInfo().m_enableSPU = true;
    mDynamicsWorld->setGravity(btVector3(0,-9.81f,0));
    */
}

PhysicsWorld::~PhysicsWorld()
{
    delete mDynamicsWorld;
    delete mSolver;
    delete mDispatcher;
    delete mCollsionConfig;
    delete mBroadphase;
}

//Start physics update task
void PhysicsWorld::StartUpdate(float deltaTime)
{
    Wait();
    mDeltaTime = deltaTime;
    mPhysicsTask = g_pMainThreadPool->AddTask(PhysicsUpdateCallback, this, 1);
    mRunning = true;
}

//Wait for physics update task
void PhysicsWorld::Wait()
{
    if (mRunning)
    {
        g_pMainThreadPool->WaitForTask(mPhysicsTask);
        mPhysicsTask = 0;
        mRunning = false;
    }
}

void PhysicsWorld::ProcessContacts()
{
    EventBodyCollide event;

    //process contacts
    int numManifolds = mDispatcher->getNumManifolds();
    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold* contactManifold = mDispatcher->getManifoldByIndexInternal(i);
        const btCollisionObject* obA = contactManifold->getBody0();
        const btCollisionObject* obB = contactManifold->getBody1();

        // extract body objects
        event.bodyA = (BodyComponent*)obA->getUserPointer();
        event.bodyB = (BodyComponent*)obB->getUserPointer();

        int numContacts = contactManifold->getNumContacts();
        for (int j = 0; j < numContacts; j++)
        {
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            if (pt.getDistance() < 0.0f)
            {
                // removed temporarly - restore when event system is improved
                //const btVector3& ptA = pt.getPositionWorldOnA();
                //const btVector3& ptB = pt.getPositionWorldOnB();
                //const btVector3& normalOnB = pt.m_normalWorldOnB;

                mScene->mEventSystem.Push(SceneEvent::BodyCollide, &event);
            }
        }
    }
}

} // namespace Scene
} // namespace NFE

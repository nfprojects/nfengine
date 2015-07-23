/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with rendering.
 */

#include "../PCH.hpp"
#include "RendererSystem.hpp"

#include "../Globals.hpp"
#include "../SceneManager.hpp"
#include "../Entity.hpp"
#include "../Engine.hpp"
#include "../BVH.hpp"
#include "../Renderer/HighLevelRenderer.hpp"
#include "../Renderer/ShadowsRenderer.hpp"
#include "../Renderer/LightsRenderer.hpp"
#include "../Renderer/GeometryBufferRenderer.hpp"
#include "../Renderer/DebugRenderer.hpp"
#include "../Renderer/RendererContext.hpp"
#include "../Renderer/View.hpp"

#include "../nfCommon/Timer.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Renderer;
using namespace Resource;


RendererSystem::RendererSystem(SceneManager* scene)
{
    mScene = scene;
}

void RendererSystem::RenderShadow(RenderContext* pCtx, LightComponent* pLight, uint32 faceID)
{
    Common::Timer timer;
    timer.Start();

    CameraComponent* pCamera = pLight->mCameras[faceID];

    CameraRenderDesc cameraRenderDesc =
    {
        pCamera->mOwner->GetMatrix(),
        pCamera->mViewMatrix,
        pCamera->mProjMatrix,
        pCamera->mSecondaryProjViewMatrix,
        pCamera->mOwner->mVelocity,
        pCamera->mOwner->mAngularVelocity,
        pCamera->mScreenScale,
        pCamera->perspective.FoV
    };

    ShadowRenderer::Get()->SetDestination(pCtx, &cameraRenderDesc, pLight->mShadowMap, faceID);

    // pCtx->commandBuffer.Clear(); // TODO

    std::vector<MeshComponent*> visibleMeshes; //TODO: dynamic allocation per frame should be avoided
    FindVisibleMeshEntities(pCamera->mFrustum, &visibleMeshes);

    //draw mesh entities
    RenderCommand command;
    command.velocity = Vector();

    for (size_t i = 0; i < visibleMeshes.size(); i++)
    {
        Mesh* pMesh = visibleMeshes[i]->mMesh;
        Vector pos = visibleMeshes[i]->mOwner->GetPosition();
        command.matrix = visibleMeshes[i]->mOwner->mMatrix;
        command.pVB = pMesh->mVB.get();
        command.pIB = pMesh->mIB.get();
        command.distance = VectorLength3(pCamera->mOwner->GetPosition() - pos).f[0];

        for (uint32 j = 0; j < pMesh->mSubMeshesCount; j++)
        {
            // TODO: submesh frustum culling
            command.indexCount = 3 * pMesh->mSubMeshes[j].trianglesCount;
            command.startIndex = pMesh->mSubMeshes[j].indexOffset;
            command.material = pMesh->mSubMeshes[j].material->GetRendererData();
            // pCtx->commandBuffer.PushBack(command); // TODO
        }
    }

    // TODO
    /*
    if (pCtx->commandBuffer.commands.size())
    {
        //sort commands
        pCtx->commandBuffer.Sort();

        //draw
        ShadowRenderer::Get()->Draw(pCtx, pCtx->commandBuffer);
    }
    */
}

// Perform Geometry Pass for pCamera in pContext
void RendererSystem::RenderGBuffer(RenderContext* pCtx, CameraComponent* pCamera)
{
    //draw mesh entities
    std::vector<MeshComponent*> visibleMeshes; //TODO: dynamic allocation per frame should be avoided
    FindVisibleMeshEntities(pCamera->mFrustum, &visibleMeshes);
    RenderCommandBuffer commandBuffer;

    RenderCommand command;
    command.velocity = Vector();
    for (size_t i = 0; i < visibleMeshes.size(); i++)
    {
        Mesh* pMesh = visibleMeshes[i]->mMesh;
        Vector pos = visibleMeshes[i]->mOwner->GetPosition();
        command.matrix = visibleMeshes[i]->mOwner->mMatrix;
        command.velocity = visibleMeshes[i]->mOwner->GetVelocity();
        command.angularVelocity = visibleMeshes[i]->mOwner->GetAngularVelocity();
        command.pVB = pMesh->mVB.get();
        command.pIB = pMesh->mIB.get();
        command.distance = VectorLength3(pCamera->mOwner->GetPosition() - pos).f[0];

        for (uint32 j = 0; j < pMesh->mSubMeshesCount; j++)
        {
            // TODO: submesh frustum culling
            command.indexCount = 3 * pMesh->mSubMeshes[j].trianglesCount;
            command.startIndex = pMesh->mSubMeshes[j].indexOffset;
            command.material = pMesh->mSubMeshes[j].material->GetRendererData();
            commandBuffer.PushBack(command);
        }
    }
    visibleMeshes.clear();
    commandBuffer.Sort();

    GBufferRenderer::Get()->Draw(pCtx, commandBuffer);
}

void RendererSystem::Render(Renderer::View* view)
{
    using namespace std::placeholders;

    CameraComponent* camera = view->GetCamera();
    RenderContext* immCtx = gRenderer->GetImmediateContext();
    immCtx->Begin();

    if (camera == nullptr)
    {
        if (mScene->mDefaultCamera)
            camera = mScene->mDefaultCamera;
        else
            return; //no default camera
    }

    camera->OnUpdate(0.0f);

    // define this, common for all renderers object, here
    CameraRenderDesc cameraRenderDesc =
    {
        camera->mOwner->GetMatrix(),
        camera->mViewMatrix,
        camera->mProjMatrix,
        camera->mSecondaryProjViewMatrix,
        camera->mOwner->mVelocity,
        camera->mOwner->mAngularVelocity,
        camera->mScreenScale,
        camera->perspective.FoV
    };


    FindActiveMeshEntities();

    //update lights
    for (auto lightTuple : mLights)
    {
        TransformComponent* transform = std::get<0>(lightTuple);
        LightComponent* light = std::get<1>(lightTuple);

        light->Update(camera);
        light->CheckShadowVisibility(camera->mOwner->GetPosition());

        //find visible lights
        if (light->mDrawShadow && light->IntersectFrustum(camera->mFrustum))
            light->mUpdateShadowmap = true;
    }

    // this value will be reused often...
    const size_t contextsNum = g_pMainThreadPool->GetThreadsNumber();

    //prepare deferred contexts
    for (size_t i = 0; i < contextsNum; i++)
    {
        RenderContext* ctx = gRenderer->GetDeferredContext(i);
        ctx->Begin();
        ShadowRenderer::Get()->Enter(ctx);
    }

    //draw shadow maps
    ShadowRenderer::Get()->Enter(immCtx);

    auto drawShadowMapFunc = [this](LightComponent * pLight, size_t instance, size_t threadID)
    {
        RenderContext* ctx = gRenderer->GetDeferredContext(threadID);
        RenderShadow(ctx, pLight, static_cast<int>(instance));
    };

    std::vector<Common::TaskID> shadowTasks;
    for (auto pLight : mLights)
    {
        if (!pLight->mUpdateShadowmap)
            continue;

        uint32 instancesCount = 1;
        if (pLight->mLightType == LightType::Omni)
            instancesCount = 6;
        else if (pLight->mLightType == LightType::Dir)
            instancesCount = pLight->mDirLight.splits;

        Common::TaskID task =
            g_pMainThreadPool->Enqueue(std::bind(drawShadowMapFunc, pLight, _1, _2),
                                       instancesCount);
        shadowTasks.push_back(task);
    }

    //execute shadowmaps rendering
    for (auto task : shadowTasks)
    {
        g_pMainThreadPool->WaitForTask(task);
    }

    auto endCommandListFunc = [] (size_t instance)
    {
        RenderContext* ctx = gRenderer->GetDeferredContext(instance);
        if (ctx)
            ctx->End();
    };

    // finish all deferred context here - build command lists
    Common::TaskID endCommandListTask = g_pMainThreadPool->Enqueue(std::bind(endCommandListFunc, _1),
                                        contextsNum);
    g_pMainThreadPool->WaitForTask(endCommandListTask);

    // execute command lists
    for (size_t i = 0; i < contextsNum; i++)
        gRenderer->ExecuteDeferredContext(gRenderer->GetDeferredContext(i));
    ShadowRenderer::Get()->Leave(immCtx);


    // TODO: this should be recorded to a deffered context as well as lights pass
    /*
    GBufferRenderer::Get()->Enter(immCtx);
    GBufferRenderer::Get()->SetUp(immCtx, view->GetGeometryBuffer());
    const float backgroundColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    immCtx->commandBuffer->Clear(NFE_CLEAR_FLAG_TARGET | NFE_CLEAR_FLAG_DEPTH, backgroundColor, 1.0f);
    GBufferRenderer::Get()->SetCamera(immCtx, &cameraRenderDesc);
    RenderGBuffer(immCtx, camera);
    GBufferRenderer::Get()->Leave(immCtx);
    */

    // LIGHTS RENDERING ===========================================================================
    {
        LightsRenderer::Get()->Enter(immCtx);
        LightsRenderer::Get()->SetUp(immCtx, view->GetRenderTarget(), &cameraRenderDesc,
                                     mScene->mEnvDesc.ambientLight, mScene->mEnvDesc.backgroundColor);

        for (auto lightTuple : mLights)
        {
            TransformComponent* transform = std::get<0>(lightTuple);
            LightComponent* light = std::get<1>(lightTuple);
            light->OnRender(immCtx);
        }

        LightsRenderer::Get()->DrawFog(immCtx);
        LightsRenderer::Get()->Leave(immCtx);
    }



    // DEBUG RENDERING ===========================================================================

    if (gRenderer->settings.debugEnable)
    {

        DebugRenderer::Get()->Enter(immCtx);
        DebugRenderer::Get()->SetCamera(immCtx, camera->mViewMatrix, camera->mProjMatrix);
        DebugRenderer::Get()->SetTarget(immCtx, view->GetRenderTarget());


        const float backgroundColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        immCtx->commandBuffer->Clear(NFE_CLEAR_FLAG_TARGET | NFE_CLEAR_FLAG_DEPTH, backgroundColor, 1.0f);
        immCtx->commandBuffer->SetRenderTarget(view->GetRenderTarget());

        //draw meshes bvh
        // DrawBVHNode(pCtx, mMeshesBVH->GetRootId(), 0);
        // TODO: removed because of "recursive call has no side effects, deleting" warning


        //draw coordinate system
        {
            DebugRenderer::Get()->DrawLine(immCtx, Float3(0, 0, 0), Float3(1.0f, 0, 0), 0x8F0000FF);
            DebugRenderer::Get()->DrawLine(immCtx, Float3(0, 0, 0), Float3(0, 1.0f, 0), 0xFF00FF00);
            DebugRenderer::Get()->DrawLine(immCtx, Float3(0, 0, 0), Float3(0, 0, 1.0f), 0xFFFF0000);
        }

        // draw entities' coordinate systems
        // TODO: entities -> transform components
        /*
        for (auto pEnt : mEntities)
        {
            Float3 start, endX, endY, endZ;
            const float debugSize = 0.2f;

            VectorStore(pEnt->mMatrix.GetRow(3), &start);
            VectorStore(debugSize * (pEnt->mMatrix.GetRow(0)) + pEnt->mMatrix.GetRow(3), &endX);
            VectorStore(debugSize * (pEnt->mMatrix.GetRow(1)) + pEnt->mMatrix.GetRow(3), &endY);
            VectorStore(debugSize * (pEnt->mMatrix.GetRow(2)) + pEnt->mMatrix.GetRow(3), &endZ);

            DebugRenderer::Get()->DrawLine(immCtx, start, endX, 0xFF0000FF);
            DebugRenderer::Get()->DrawLine(immCtx, start, endY, 0xFF00FF00);
            DebugRenderer::Get()->DrawLine(immCtx, start, endZ, 0xFFFF0000);
        }
        */

        // draw light shapes
        if (gRenderer->settings.debugLights)
        {
            for (auto lightTuple : mLights)
            {
                TransformComponent* transform = std::get<0>(lightTuple);
                LightComponent* light = std::get<1>(lightTuple);
                light->OnRenderDebug(immCtx);
            }
        }

        //draw meshes AABBs
        if (gRenderer->settings.debugMeshes)
        {
            for (size_t i = 0; i < mActiveMeshEntities.size(); i++)
                mActiveMeshEntities[i]->OnRenderDebug(immCtx);
        }

        DebugRenderer::Get()->Leave(immCtx);
    }
}

//find entities that have mesh bound
void RendererSystem::FindActiveMeshEntities()
{
    mActiveMeshEntities.clear();
    //mMeshesBVH->Clear();

#ifdef USE_BVH
    bool updateBVH = static_cast<bool>(mMeshesBVH->GetSize() == 0);
#endif

    for (auto pMeshComp : mMeshes)
    {
        Mesh* pMesh = pMeshComp->mMesh;
        if (pMesh == 0) continue;
        if (pMesh->GetState() != ResourceState::Loaded) continue;

        pMeshComp->CalcAABB();
        mActiveMeshEntities.push_back(pMeshComp);

#ifdef USE_BVH
        // insert to the BVH
        if (updateBVH)
            mMeshesBVH->Insert(pMeshComp->mGlobalAABB, pMeshComp);
#endif
    }

#ifdef USE_BVH
    if (updateBVH)
    {
        BVHStats stats;
        mMeshesBVH->GetStats(&stats);

        char str[128];
        sprintf_s(str,
                  "!!! NFEngine: Leaves num: %u, nodes num: %u, tree height: %u, area: %f, volume: %f\n",
                  stats.leavesNum, stats.nodesNum, stats.height, stats.totalArea, stats.totatlVolume);
        OutputDebugStringA(str);
    }
#endif
}

// Callback used by BVH query
void MeshQueryCallback(void* pLeafUserData, void* pCallbackUserData)
{
    auto pMeshComp = (MeshComponent*)pLeafUserData;
    auto pList = (std::vector<MeshComponent*>*)pCallbackUserData;
    pList->push_back(pMeshComp);
}

// build list of meshes visible in a frustum
void RendererSystem::FindVisibleMeshEntities(const Frustum& frustum,
                                             std::vector<MeshComponent*>* pList)
{
#ifdef USE_BVH
    mMeshesBVH->Query(frustum, MeshQueryCallback, pList);
#else

    for (const auto& mesh : mActiveMeshEntities)
    {
        if (Intersect(mesh->mGlobalAABB, frustum))
            pList->push_back(mesh);
    }
#endif
}

void RendererSystem::Update(float dt)
{
    mMeshes.clear();
    mCameras.clear();
    mLights.clear();

    // segregate componets
    for (auto pEntity : mScene->mEntities)
    {
        for (auto pComp : pEntity->mComponents)
        {
            switch (pComp->GetType())
            {
                case ComponentType::Mesh:
                {
                    mMeshes.push_back((MeshComponent*)pComp);
                    break;
                }
                case ComponentType::Light:
                {
                    LightComponent* light = (LightComponent*)pComp;
                    light->mUpdateShadowmap = false;
                    mLights.push_back((LightComponent*)pComp);
                    break;
                }
                case ComponentType::CameraComponent:
                {
                    mCameras.push_back((CameraComponent*)pComp);
                    break;
                }
            }
        }
    }
}

} // namespace Scene
} // namespace NFE

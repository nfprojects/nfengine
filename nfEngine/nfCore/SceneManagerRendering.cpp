/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with rendering.
 */

#include "PCH.hpp"
#include "Globals.hpp"
#include "SceneManager.hpp"
#include "Entity.hpp"
#include "Performance.hpp"
#include "Engine.hpp"
#include "BVH.hpp"
#include "Renderer/HighLevelRenderer.hpp"
#include "Renderer/ShadowsRenderer.hpp"
#include "Renderer/LightsRenderer.hpp"
#include "Renderer/GeometryBufferRenderer.hpp"
#include "Renderer/DebugRenderer.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/View.hpp"
#include "../nfCommon/Timer.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Renderer;
using namespace Resource;

void SceneManager::RenderShadow(RenderContext* pCtx, LightComponent* pLight, uint32 faceID)
{
    Common::Timer timer;
    timer.Start();

    Camera* pCamera = pLight->mCameras[faceID];

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
            //Box globalBox = TransformBox(command.matrix, command.pMesh->mSubMeshes[j].mLocalBox);
            //if (IntersectBoxFrustum(globalBox, pCamera->mFrustum))
            {
                command.indexCount = 3 * pMesh->mSubMeshes[j].trianglesCount;
                command.startIndex = pMesh->mSubMeshes[j].indexOffset;
                command.material = pMesh->mSubMeshes[j].material->GetRendererData();
                // pCtx->commandBuffer.PushBack(command); // TODO
                Util::g_FrameStats.renderedMeshes++;
            }
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
void SceneManager::RenderGBuffer(RenderContext* pCtx, Camera* pCamera)
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
            //Box globalBox = TransformBox(command.matrix, command.pMesh->mSubMeshes[j].mLocalBox);
            //if (IntersectBoxFrustum(globalBox, pCamera->mFrustum))
            {
                command.indexCount = 3 * pMesh->mSubMeshes[j].trianglesCount;
                command.startIndex = pMesh->mSubMeshes[j].indexOffset;
                command.material = pMesh->mSubMeshes[j].material->GetRendererData();
                commandBuffer.PushBack(command);
                Util::g_FrameStats.renderedMeshes++;
            }
        }
    }
    visibleMeshes.clear();
    commandBuffer.Sort();

    GBufferRenderer::Get()->Draw(pCtx, commandBuffer);
}


void SceneManager::DrawBVHNode(RenderContext* pCtx, uint32 node, uint32 depth)
{
    if (node == 0xFFFFFFFF) return;
    Util::BVHNode* pNode = mMeshesBVH->GetNodeById(node);

    DrawBVHNode(pCtx, pNode->child[0], depth + 1);
    DrawBVHNode(pCtx, pNode->child[1], depth + 1);

    uint32 color = 0x7F000000 + depth;
    DebugRenderer::Get()->DrawBox(pCtx, pNode->AABB, color);
}

void SceneManager::Render(Renderer::View* view)
{
    using namespace std::placeholders;

    Camera* camera = view->GetCamera();
    RenderContext* immCtx = gRenderer->GetImmediateContext();
    immCtx->Begin();

    if (camera == 0)
    {
        if (mDefaultCamera)
            camera = mDefaultCamera;
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
    for (auto pLight : mLights)
    {
        pLight->Update(camera);
        pLight->CheckShadowVisibility(camera->mOwner->GetPosition());

        //find visible lights
        if (pLight->mDrawShadow && pLight->IntersectFrustum(camera->mFrustum))
            pLight->mUpdateShadowmap = true;
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

    auto drawShadowMapFunc = [](LightComponent * pLight, size_t instance, size_t threadID)
    {
        RenderContext* ctx = gRenderer->GetDeferredContext(threadID);
        pLight->mOwner->GetScene()->RenderShadow(ctx, pLight, static_cast<int>(instance));
    };

    std::vector<Common::TaskID> shadowTasks;
    for (auto pLight : mLights)
    {
        if (!pLight->mUpdateShadowmap) continue;

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
    GBufferRenderer::Get()->Enter(immCtx);
    GBufferRenderer::Get()->SetUp(immCtx, view->GetGeometryBuffer());
    const float backgroundColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    immCtx->commandBuffer->Clear(NFE_CLEAR_FLAG_TARGET | NFE_CLEAR_FLAG_DEPTH, backgroundColor, 1.0f);
    GBufferRenderer::Get()->SetCamera(immCtx, &cameraRenderDesc);
    RenderGBuffer(immCtx, camera);
    GBufferRenderer::Get()->Leave(immCtx);

    // LIGHTS RENDERING ===========================================================================
    {
        LightsRenderer::Get()->Enter(immCtx);
        LightsRenderer::Get()->SetUp(immCtx, view->GetRenderTarget(), view->GetGeometryBuffer(),
                                     &cameraRenderDesc);
        LightsRenderer::Get()->DrawAmbientLight(immCtx, mEnvDesc.ambientLight, mEnvDesc.backgroundColor);

        if (gRenderer->settings.tileBasedDeferredShading)
        {
            //prepare lights for tile based renderimng
            std::vector<TileOmniLightDesc> omniLights;
            for (auto pLight : mLights)
            {
                if (pLight->CanBeTiled())
                {
                    TileOmniLightDesc desc;
                    VectorStore(pLight->mOwner->GetPosition(), &desc.pos);
                    VectorStore(pLight->mColor, &desc.color);
                    desc.radius = pLight->mOmniLight.radius;
                    desc.radiusInv = 1.0f / desc.radius;
                    omniLights.push_back(desc);
                }
            }

            if (omniLights.size() > 0)
                LightsRenderer::Get()->TileBasedPass(immCtx, static_cast<uint32>(omniLights.size()),
                                                &omniLights[0]);
            else
                LightsRenderer::Get()->TileBasedPass(immCtx, 0, 0);


            //draw remaining lights
            for (auto pLight : mLights)
            {
                if (!pLight->CanBeTiled())
                {
                    pLight->OnRender(immCtx);
                }
            }
        }
        else
        {
            for (auto pLight : mLights)
            {
                pLight->OnRender(immCtx);
            }
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

        // draw light shapes
        if (gRenderer->settings.debugLights)
        {
            for (auto pLight : mLights)
                pLight->OnRenderDebug(immCtx);
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

} // namespace Scene
} // namespace NFE

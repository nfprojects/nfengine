/**
    NFEngine project

    \file   SceneManagerRendering.cpp
    \brief  Definitions of Scene class methods connected only with rendering.
*/

#include "stdafx.h"
#include "Globals.h"
#include "SceneManager.h"
#include "Entity.h"
#include "Performance.h"
#include "Engine.h"
#include "BVH.h"
#include "Renderer.h"
#include "../nfCommon/Timer.hpp"
#include "../nfCommon/Logger.h"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Render;
using namespace Resource;

void SceneManager::RenderShadow(IRenderContext* pCtx, LightComponent* pLight, uint32 faceID)
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

    g_pShadowRenderer->SetDestination(pCtx, &cameraRenderDesc, pLight->mShadowMap, faceID);

    pCtx->commandBuffer.Clear();

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
        command.pVB = pMesh->mVB;
        command.pIB = pMesh->mIB;
        command.distance = VectorLength3(pCamera->mOwner->GetPosition() - pos).f[0];

        for (uint32 j = 0; j < pMesh->mSubMeshesCount; j++)
        {
            //Box globalBox = TransformBox(command.matrix, command.pMesh->mSubMeshes[j].mLocalBox);
            //if (IntersectBoxFrustum(globalBox, pCamera->mFrustum))
            {
                command.indexCount = 3 * pMesh->mSubMeshes[j].trianglesCount;
                command.startIndex = pMesh->mSubMeshes[j].indexOffset;
                command.pMaterial = pMesh->mSubMeshes[j].material->GetRendererData();
                pCtx->commandBuffer.PushBack(command);
                Util::g_FrameStats.renderedMeshes++;
            }
        }
    }

    if (pCtx->commandBuffer.commands.size())
    {
        //sort commands
        pCtx->commandBuffer.Sort();

        //draw
        g_pShadowRenderer->Draw(pCtx, pCtx->commandBuffer);
    }
}

// Perform Geometry Pass for pCamera in pContext
void SceneManager::RenderGBuffer(IRenderContext* pCtx, Camera* pCamera,
                                 CameraRenderDesc* pCameraDesc, IRenderTarget* pRT)
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
        command.pVB = pMesh->mVB;
        command.pIB = pMesh->mIB;
        command.distance = VectorLength3(pCamera->mOwner->GetPosition() - pos).f[0];

        for (uint32 j = 0; j < pMesh->mSubMeshesCount; j++)
        {
            //Box globalBox = TransformBox(command.matrix, command.pMesh->mSubMeshes[j].mLocalBox);
            //if (IntersectBoxFrustum(globalBox, pCamera->mFrustum))
            {
                command.indexCount = 3 * pMesh->mSubMeshes[j].trianglesCount;
                command.startIndex = pMesh->mSubMeshes[j].indexOffset;
                command.pMaterial = pMesh->mSubMeshes[j].material->GetRendererData();
                commandBuffer.PushBack(command);
                Util::g_FrameStats.renderedMeshes++;
            }
        }
    }
    visibleMeshes.clear();
    commandBuffer.Sort();


    g_pGBufferRenderer->Enter(pCtx);
    g_pGBufferRenderer->SetTarget(pCtx, pRT);
    g_pGBufferRenderer->SetCamera(pCtx, pCameraDesc);
    g_pGBufferRenderer->Draw(pCtx, commandBuffer);
    g_pGBufferRenderer->Leave(pCtx);
}


void SceneManager::DrawBVHNode(IRenderContext* pCtx, uint32 node, uint32 depth)
{
    if (node == 0xFFFFFFFF) return;
    Util::BVHNode* pNode = mMeshesBVH->GetNodeById(node);

    DrawBVHNode(pCtx, pNode->child[0], depth + 1);
    DrawBVHNode(pCtx, pNode->child[1], depth + 1);

    uint32 color = 0x7F000000 + depth;
    g_pDebugRenderer->DrawBox(pCtx, pNode->AABB, color);
}

void SceneManager::Render(Camera* pCamera, IRenderTarget* pRT)
{
    using namespace std::placeholders;

    g_pImmediateContext->Begin();

    if (pCamera == 0)
    {
        if (mDefaultCamera)
            pCamera = mDefaultCamera;
        else
            return; //no default camera
    }

    pCamera->OnUpdate(0.0f);

    // define this, common for all renderers object, here
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


    FindActiveMeshEntities();

    //update lights
    for (auto pLight : mLights)
    {
        pLight->Update(pCamera);
        pLight->CheckShadowVisibility(pCamera->mOwner->GetPosition());

        //find visible lights
        if (pLight->mDrawShadow && pLight->IntersectFrustum(pCamera->mFrustum))
            pLight->mUpdateShadowmap = true;
    }

    // this value will be reused often...
    const size_t contextsNum = g_pMainThreadPool->GetThreadsNumber();

    //prepare deferred contexts
    for (size_t i = 0; i < contextsNum; i++)
    {
        g_pDeferredContexts[i]->Begin();
        g_pShadowRenderer->Enter(g_pDeferredContexts[i]);
    }

    //draw shadow maps
    g_pShadowRenderer->Enter(g_pImmediateContext);

    auto drawShadowMapFunc = [](LightComponent * pLight, size_t instance, size_t threadID)
    {
        IRenderContext* pContext = g_pDeferredContexts[threadID];
        pLight->mOwner->GetScene()->RenderShadow(pContext, pLight, static_cast<int>(instance));
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
        IRenderContext* pContext = g_pDeferredContexts[instance];
        if (pContext)
            pContext->End();
    };

    // finish all deferred context here - build command lists
    Common::TaskID endCommandListTask = g_pMainThreadPool->Enqueue(std::bind(endCommandListFunc, _1),
                                        contextsNum);
    g_pMainThreadPool->WaitForTask(endCommandListTask);

    // execute command lists
    for (size_t i = 0; i < contextsNum; i++)
        g_pRenderer->ExecuteDeferredContext(g_pDeferredContexts[i]);
    g_pShadowRenderer->Leave(g_pImmediateContext);


    // TODO: this should be recorded to a deffered context as well as lights pass
    RenderGBuffer(g_pImmediateContext, pCamera, &cameraRenderDesc, pRT);


    // LIGHTS RENDERING ===========================================================================
    {
        IRenderContext* pCtx = g_pImmediateContext;

        g_pLightRenderer->Enter(pCtx);
        g_pLightRenderer->SetUp(pCtx, pRT, &cameraRenderDesc, mEnvDesc.ambientLight,
                                mEnvDesc.backgroundColor);

        if (g_pRenderer->settings.tileBasedDeferredShading)
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
                g_pLightRenderer->TileBasedPass(pCtx, omniLights.size(), &omniLights[0]);
            else
                g_pLightRenderer->TileBasedPass(pCtx, 0, 0);


            //draw remaining lights
            for (auto pLight : mLights)
            {
                if (!pLight->CanBeTiled())
                {
                    pLight->OnRender(pCtx);
                }
            }
        }
        else
        {
            for (auto pLight : mLights)
            {
                pLight->OnRender(pCtx);
            }
        }

        g_pLightRenderer->DrawFog(pCtx);
        g_pLightRenderer->Leave(pCtx);
    }



    // DEBUG RENDERING ===========================================================================

    if (g_pRenderer->settings.debugEnable)
    {
        IRenderContext* pCtx = g_pImmediateContext;

        g_pDebugRenderer->Enter(g_pImmediateContext);
        g_pDebugRenderer->SetCamera(pCtx, pCamera->mViewMatrix, pCamera->mProjMatrix);


        //draw meshes bvh
        DrawBVHNode(pCtx, mMeshesBVH->GetRootId(), 0);


        //draw coordinate system
        {
            g_pDebugRenderer->DrawLine(pCtx, Float3(0, 0, 0), Float3(1.0f, 0, 0), 0x8F0000FF);
            g_pDebugRenderer->DrawLine(pCtx, Float3(0, 0, 0), Float3(0, 1.0f, 0), 0xFF00FF00);
            g_pDebugRenderer->DrawLine(pCtx, Float3(0, 0, 0), Float3(0, 0, 1.0f), 0xFFFF0000);
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

            g_pDebugRenderer->DrawLine(pCtx, start, endX, 0xFF0000FF);
            g_pDebugRenderer->DrawLine(pCtx, start, endY, 0xFF00FF00);
            g_pDebugRenderer->DrawLine(pCtx, start, endZ, 0xFFFF0000);
        }

        // draw light shapes
        if (g_pRenderer->settings.debugLights)
        {
            for (auto pLight : mLights)
                pLight->OnRenderDebug(pCtx);
        }

        //draw meshes AABBs
        if (g_pRenderer->settings.debugMeshes)
        {
            for (size_t i = 0; i < mActiveMeshEntities.size(); i++)
                mActiveMeshEntities[i]->OnRenderDebug(pCtx);
        }

        g_pDebugRenderer->Leave(pCtx);
    }
}

} // namespace Scene
} // namespace NFE

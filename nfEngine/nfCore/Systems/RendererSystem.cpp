/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with rendering.
 */

#include "../PCH.hpp"
#include "RendererSystem.hpp"

#include "../EntityManager.hpp"
#include "../Engine.hpp"
#include "../Material.hpp"

#include "../Renderer/HighLevelRenderer.hpp"
#include "../Renderer/ShadowsRenderer.hpp"
#include "../Renderer/LightsRenderer.hpp"
#include "../Renderer/GeometryBufferRenderer.hpp"
#include "../Renderer/DebugRenderer.hpp"
#include "../Renderer/RendererContext.hpp"
#include "../Renderer/View.hpp"

#include "../Components/TransformComponent.hpp"
#include "../Components/BodyComponent.hpp"
#include "../Components/MeshComponent.hpp"
#include "../Components/CameraComponent.hpp"
#include "../Components/LightComponent.hpp"

#include "../nfCommon/Timer.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/Math/Geometry.hpp"
#include "../nfCommon/Math/Sphere.hpp"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Renderer;
using namespace Resource;

RendererSystem::RendererSystem(SceneManager* scene)
    : mScene(scene)
{
}

void RendererSystem::RenderLights(RenderContext* ctx) const
{
    for (auto lightTuple : mOmniLights)
    {
        TransformComponent* transform = std::get<0>(lightTuple);
        LightComponent* light = std::get<1>(lightTuple);

        LightsRenderer::Get()->DrawOmniLight(ctx, transform->GetPosition(),
                                             light->mOmniLight.radius, light->mColor,
                                             light->mShadowMap);
    }

    for (size_t i = 0; i < mSpotLights.size(); ++i)
    {
        const SpotLightData& lightData = mSpotLightsData[i];
        const TransformComponent* transform = std::get<0>(mSpotLights[i]);
        const LightComponent* light = std::get<1>(mSpotLights[i]);

        SpotLightProperties prop;
        prop.color = light->mColor;
        prop.position = transform->GetPosition();
        prop.direction = transform->GetMatrix().r[2];
        prop.farDist.Set(light->mSpotLight.farDist);
        prop.viewProjMatrix = lightData.viewMatrix * lightData.projMatrix;
        prop.viewProjMatrixInv = MatrixInverse(prop.viewProjMatrix); // TODO move to SpotLightData?

        ITexture* lightMap = light->mLightMap ? light->mLightMap->GetRendererTexture() : nullptr;

        LightsRenderer::Get()->DrawSpotLight(ctx, prop, nullptr, lightMap);
    }

    for (auto lightTuple : mDirLights)
    {
        TransformComponent* transform = std::get<0>(lightTuple);
        LightComponent* light = std::get<1>(lightTuple);

        DirLightProperties prop;
        prop.color = light->mColor;
        prop.direction = transform->GetMatrix().GetRow(2);
        prop.cascadesCount[0] = light->mDirLight.splits;

        for (int i = 0; i < light->mDirLight.splits; i++)
        {
            // TODO: calculate view and projection matrices
            prop.splitDistance[i] = Vector(); // TODO
            prop.viewProjMatrix[i] = Matrix();
        }

        LightsRenderer::Get()->DrawDirLight(ctx, prop, light->mShadowMap);
    }
}

void RendererSystem::RenderLightsDebug(Renderer::RenderContext* ctx) const
{
    uint32 lightDebugColor = 0xFF00FFFF;

    for (auto lightTuple : mOmniLights)
    {
        TransformComponent* transform = std::get<0>(lightTuple);
        LightComponent* light = std::get<1>(lightTuple);

        Box box;
        box.min = transform->GetPosition() - VectorSplat(light->mOmniLight.radius);
        box.max = transform->GetPosition() + VectorSplat(light->mOmniLight.radius);
        DebugRenderer::Get()->DrawBox(ctx, box, lightDebugColor);
    }

    for (size_t i = 0; i < mSpotLights.size(); ++i)
    {
        DebugRenderer::Get()->DrawFrustum(ctx, mSpotLightsData[i].frustum, lightDebugColor);
    }

    // TODO: dir light
}

void RendererSystem::RenderShadow(RenderContext* pCtx, LightComponent* pLight, uint32 faceID)
{
    Common::Timer timer;
    timer.Start();

    /*
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

    pCtx->commandBuffer.Clear(); // TODO

    std::vector<MeshComponent*> visibleMeshes;
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

    if (pCtx->commandBuffer.commands.size())
    {
        //sort commands
        pCtx->commandBuffer.Sort();

        //draw
        ShadowRenderer::Get()->Draw(pCtx, pCtx->commandBuffer);
    }
    */
}

void RendererSystem::RenderGBuffer(RenderContext* ctx, CameraComponent* camera,
                                   TransformComponent* cameraTransform)
{
    //draw mesh entities
    std::vector<MeshEntry> visibleMeshes; //TODO: dynamic allocation per frame should be avoided
    FindVisibleMeshEntities(camera->mFrustum, visibleMeshes);
    RenderCommandBuffer commandBuffer;

    for (auto meshTuple : visibleMeshes)
    {
        TransformComponent* transform = std::get<0>(meshTuple);
        MeshComponent* mesh = std::get<1>(meshTuple);
        BodyComponent* body = std::get<2>(meshTuple);
        Mesh* meshResource = mesh->mMesh;

        RenderCommand command;
        command.matrix = transform->GetMatrix();
        if (body != nullptr)
        {
            command.velocity = body->GetVelocity();
            command.angularVelocity = body->GetAngularVelocity();
        }
        else
        {
            command.velocity = Vector();
            command.angularVelocity = Vector();
        }
        command.pVB = meshResource->mVB.get();
        command.pIB = meshResource->mIB.get();
        command.distance = VectorLength3(cameraTransform->GetPosition() -
                                         transform->GetPosition()).f[0];

        for (uint32 j = 0; j < meshResource->mSubMeshesCount; j++)
        {
            // TODO: submesh frustum culling
            command.indexCount = 3 * meshResource->mSubMeshes[j].trianglesCount;
            command.startIndex = meshResource->mSubMeshes[j].indexOffset;
            command.material = meshResource->mSubMeshes[j].material->GetRendererData();
            commandBuffer.PushBack(command);
        }
    }
    visibleMeshes.clear();
    commandBuffer.Sort();

    GBufferRenderer::Get()->Draw(ctx, commandBuffer);
}

void RendererSystem::Render(Renderer::View* view)
{
    using namespace std::placeholders;

    EntityManager* entityManager = mScene->GetEntityManager();
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    EntityID cameraEntity = view->GetCameraEntity();
    auto camera = entityManager->GetComponent<CameraComponent>(cameraEntity);
    auto cameraTransform = entityManager->GetComponent<TransformComponent>(cameraEntity);
    auto cameraBody = entityManager->GetComponent<BodyComponent>(cameraEntity);

    RenderContext* immCtx = renderer->GetImmediateContext();
    immCtx->Begin();

    Vector cameraVelocity;
    Vector cameraAngularVelocity;
    if (cameraBody != nullptr)
    {
        cameraVelocity = cameraBody->GetVelocity();
        cameraAngularVelocity = cameraBody->GetAngularVelocity();
    }

    // TODO: velocity
    camera->Update(cameraTransform->GetMatrix(), cameraVelocity, cameraAngularVelocity, 0.0f);

    // define this, common for all renderers object, here
    CameraRenderDesc cameraRenderDesc =
    {
        cameraTransform->GetMatrix(),
        camera->mViewMatrix,
        camera->mProjMatrix,
        camera->mSecondaryProjViewMatrix,
        cameraVelocity,
        cameraAngularVelocity,
        camera->mScreenScale,
        camera->mPerspective.FoV
    };

    FindActiveMeshEntities();

    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();
    const size_t contextsNum = threadPool->GetThreadsNumber();

    // prepare deferred contexts
    for (size_t i = 0; i < contextsNum; i++)
    {
        RenderContext* ctx = renderer->GetDeferredContext(i);
        ctx->Begin();
        ShadowRenderer::Get()->Enter(ctx);
    }

    // draw shadow maps
    ShadowRenderer::Get()->Enter(immCtx);

    auto drawShadowMapFunc = [this, &renderer](LightComponent * pLight, size_t instance,
                                               size_t threadID)
    {
        RenderContext* ctx = renderer->GetDeferredContext(threadID);
        RenderShadow(ctx, pLight, static_cast<int>(instance));
    };

    std::vector<Common::TaskID> shadowTasks;

    /*
    for (auto lightTuple : mLights)
    {
        LightComponent* light = std::get<1>(lightTuple);

        if (!light->mDrawShadow)
            continue;

        uint32 instancesCount = 1;
        if (light->mLightType == LightType::Omni)
            instancesCount = 6;
        else if (light->mLightType == LightType::Dir)
            instancesCount = light->mDirLight.splits;

        Common::TaskID task =
            threadPool->Enqueue(std::bind(drawShadowMapFunc, light, _1, _2), instancesCount);
        shadowTasks.push_back(task);
    }
    */

    // execute shadowmaps rendering
    for (auto task : shadowTasks)
    {
        threadPool->WaitForTask(task);
    }

    auto endCommandListFunc = [&renderer] (size_t instance)
    {
        RenderContext* ctx = renderer->GetDeferredContext(instance);
        if (ctx)
            ctx->End();
    };

    // finish all deferred context here - build command lists
    Common::TaskID endCommandListTask = threadPool->Enqueue(std::bind(endCommandListFunc, _1),
                                                            contextsNum);
    threadPool->WaitForTask(endCommandListTask);

    // execute command lists
    for (size_t i = 0; i < contextsNum; i++)
        renderer->ExecuteDeferredContext(renderer->GetDeferredContext(i));
    ShadowRenderer::Get()->Leave(immCtx);


    // TODO: this should be recorded to a deffered context as well as lights pass
    GBufferRenderer::Get()->Enter(immCtx);
    GBufferRenderer::Get()->SetUp(immCtx, view->GetGeometryBuffer());
    const float backgroundColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    immCtx->commandBuffer->Clear(NFE_CLEAR_FLAG_TARGET | NFE_CLEAR_FLAG_DEPTH, backgroundColor, 1.0f);
    GBufferRenderer::Get()->SetCamera(immCtx, &cameraRenderDesc);
    RenderGBuffer(immCtx, camera, cameraTransform);
    GBufferRenderer::Get()->Leave(immCtx);

    // LIGHTS RENDERING ===========================================================================
    {
        LightsRenderer::Get()->Enter(immCtx);
        LightsRenderer::Get()->SetUp(immCtx, view->GetRenderTarget(), view->GetGeometryBuffer(),
                                     &cameraRenderDesc);
        LightsRenderer::Get()->DrawAmbientLight(immCtx,
                                                Vector(0.4f, 0.5f, 0.6f),
                                                Vector(0.2f, 0.25f, 0.3f));
        RenderLights(immCtx);

        LightsRenderer::Get()->DrawFog(immCtx);
        LightsRenderer::Get()->Leave(immCtx);
    }



    // DEBUG RENDERING ===========================================================================

    if (renderer->settings.debugEnable)
    {
        DebugRenderer::Get()->Enter(immCtx);
        DebugRenderer::Get()->SetCamera(immCtx, camera->mViewMatrix, camera->mProjMatrix);
        DebugRenderer::Get()->SetTarget(immCtx, view->GetRenderTarget());

        //draw coordinate system
        {
            DebugRenderer::Get()->DrawLine(immCtx, Float3(0, 0, 0), Float3(1.0f, 0, 0), 0x8F0000FF);
            DebugRenderer::Get()->DrawLine(immCtx, Float3(0, 0, 0), Float3(0, 1.0f, 0), 0xFF00FF00);
            DebugRenderer::Get()->DrawLine(immCtx, Float3(0, 0, 0), Float3(0, 0, 1.0f), 0xFFFF0000);
        }

        // draw entities' coordinate systems
        for (auto meshTuple : mActiveMeshEntities)
        {
            TransformComponent* transform = std::get<0>(meshTuple);

            Float3 start, endX, endY, endZ;
            const float debugSize = 0.2f;

            Matrix matrix = transform->GetMatrix();
            VectorStore(matrix.GetRow(3), &start);
            VectorStore(debugSize * (matrix.GetRow(0)) + matrix.GetRow(3), &endX);
            VectorStore(debugSize * (matrix.GetRow(1)) + matrix.GetRow(3), &endY);
            VectorStore(debugSize * (matrix.GetRow(2)) + matrix.GetRow(3), &endZ);

            DebugRenderer::Get()->DrawLine(immCtx, start, endX, 0xFF0000FF);
            DebugRenderer::Get()->DrawLine(immCtx, start, endY, 0xFF00FF00);
            DebugRenderer::Get()->DrawLine(immCtx, start, endZ, 0xFFFF0000);
        }

        // draw light shapes
        if (renderer->settings.debugLights)
        {
            RenderLightsDebug(immCtx);
        }

        // draw meshes AABBs
        if (renderer->settings.debugMeshes)
        {
            for (auto meshTuple : mActiveMeshEntities)
            {
                TransformComponent* transform = std::get<0>(meshTuple);
                MeshComponent* mesh = std::get<1>(meshTuple);

                // TODO: change to boxes only
                DebugRenderer::Get()->DrawMesh(immCtx, mesh->mMesh, transform->GetMatrix());
            }
        }

        DebugRenderer::Get()->Leave(immCtx);
    }
}

// find entities that have mesh bound
void RendererSystem::FindActiveMeshEntities()
{
    mActiveMeshEntities.clear();

    for (auto meshTuple : mMeshes)
    {
        TransformComponent* transform = std::get<0>(meshTuple);
        MeshComponent* mesh = std::get<1>(meshTuple);

        Mesh* meshResource = mesh->mMesh;
        if (meshResource == 0)
            continue;
        if (meshResource->GetState() != ResourceState::Loaded)
            continue;

        mesh->CalcAABB(transform->GetMatrix());
        mActiveMeshEntities.push_back(meshTuple);
    }
}

// build list of meshes visible in a frustum
void RendererSystem::FindVisibleMeshEntities(const Frustum& frustum, std::vector<MeshEntry>& list)
{
    for (auto meshTuple : mActiveMeshEntities)
    {
        MeshComponent* mesh = std::get<1>(meshTuple);

        if (Intersect(mesh->mGlobalAABB, frustum))
            list.push_back(meshTuple);
    }
}

void RendererSystem::UpdateLights()
{
    EntityManager* entityManager = mScene->GetEntityManager();

    mOmniLights.clear();
    mSpotLights.clear();
    mDirLights.clear();

    entityManager->ForEach<TransformComponent, LightComponent>(
        [&](EntityID entity,
            TransformComponent* transform,
            LightComponent* light)
        {
            switch (light->mLightType)
            {
            case LightType::Omni:
                mOmniLights.push_back(std::make_tuple(transform, light));
                break;
            case LightType::Spot:
                mSpotLights.push_back(std::make_tuple(transform, light));
                break;
            case LightType::Dir:
                mDirLights.push_back(std::make_tuple(transform, light));
                break;
            }
        }
    );

    mSpotLightsData.resize(mSpotLights.size());

    for (size_t i = 0; i < mSpotLights.size(); ++i)
    {
        TransformComponent* transform = std::get<0>(mSpotLights[i]);
        LightComponent* light = std::get<1>(mSpotLights[i]);

        float nearD = light->mSpotLight.nearDist;
        float farD = light->mSpotLight.farDist;
        float scale = tanf(light->mSpotLight.cutoff / 2.0f);

        Vector xAxis = scale * transform->GetMatrix().GetRow(0);
        Vector yAxis = scale * transform->GetMatrix().GetRow(1);
        Vector zAxis = scale * transform->GetMatrix().GetRow(2);

        Vector pos = transform->GetPosition();

        SpotLightData& data = mSpotLightsData[i];

        data.frustum.verticies[0] = pos + nearD * (zAxis - xAxis - yAxis);
        data.frustum.verticies[1] = pos + nearD * (zAxis + xAxis - yAxis);
        data.frustum.verticies[2] = pos + nearD * (zAxis - xAxis + yAxis);
        data.frustum.verticies[3] = pos + nearD * (zAxis + xAxis + yAxis);
        data.frustum.verticies[4] = pos + farD * (zAxis - xAxis - yAxis);
        data.frustum.verticies[5] = pos + farD * (zAxis + xAxis - yAxis);
        data.frustum.verticies[6] = pos + farD * (zAxis - xAxis + yAxis);
        data.frustum.verticies[7] = pos + farD * (zAxis + xAxis + yAxis);
        data.frustum.CalculatePlanes();

        // calculate view and projection matrices
        data.viewMatrix = MatrixLookTo(pos,
                                       transform->GetMatrix().GetRow(2),
                                       transform->GetMatrix().GetRow(1));
        data.projMatrix = MatrixPerspective(1.0f,  // aspect ratio
                                            light->mSpotLight.cutoff,
                                            light->mSpotLight.farDist,
                                            light->mSpotLight.nearDist);
    }
}

void RendererSystem::Update(float dt)
{
    EntityManager* entityManager = mScene->GetEntityManager();

    mMeshes.clear();
    mCameras.clear();

    entityManager->ForEach<TransformComponent, MeshComponent>(
        [&](EntityID entity,
            TransformComponent* transform,
            MeshComponent* mesh)
        {
            BodyComponent* body = entityManager->GetComponent<BodyComponent>(entity);
            mMeshes.push_back(std::make_tuple(transform, mesh, body));
        }
    );

    entityManager->ForEach<TransformComponent, CameraComponent>(
        [&](EntityID entity,
            TransformComponent* transform,
            CameraComponent* camera)
        {
            BodyComponent* body = entityManager->GetComponent<BodyComponent>(entity);
            mCameras.push_back(std::make_tuple(transform, camera, body));
        }
    );

    // TODO: this can be done on a separate thread
    UpdateLights();
}

} // namespace Scene
} // namespace NFE

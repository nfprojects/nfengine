/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with rendering.
 */

#include "PCH.hpp"
#include "RendererSystem.hpp"

#include "Scene/EntityManager.hpp"
#include "Engine.hpp"
#include "Resources/Material.hpp"

#include "Renderer/HighLevelRenderer.hpp"
#include "Renderer/LightsRenderer.hpp"
#include "Renderer/GeometryRenderer.hpp"
#include "Renderer/DebugRenderer.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/View.hpp"

#include "Components/TransformComponent.hpp"
#include "Components/BodyComponent.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/LightComponent.hpp"

#include "nfCommon/Timer.hpp"
#include "nfCommon/Logger.hpp"
#include "nfCommon/Math/Geometry.hpp"
#include "nfCommon/Math/Sphere.hpp"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Renderer;
using namespace Resource;

RenderingData::RenderingData()
    : view(nullptr)
    , cameraComponent(nullptr)
    , cameraTransform(nullptr)
    , sceneRenderTask(NFE_INVALID_TASK_ID)
    , shadowPassTask(NFE_INVALID_TASK_ID)
    , geometryPassTask(NFE_INVALID_TASK_ID)
    , lightsPassTask(NFE_INVALID_TASK_ID)
    , debugLayerTask(NFE_INVALID_TASK_ID)
{
}

RenderingData::RenderingData(const RenderingData& other)
{
}

void RenderingData::ExecuteCommandLists() const
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    // wait for the main renderer task - it spawns the rest of tasks used here...
    if (sceneRenderTask != NFE_INVALID_TASK_ID)
        threadPool->WaitForTask(sceneRenderTask);

    // execute collected shadow command lists
    if (shadowPassTask != NFE_INVALID_TASK_ID)
    {
        threadPool->WaitForTask(shadowPassTask);
        for (const auto& commandLists : shadowPassCLs)
            for (const auto& commandList : commandLists)
                renderer->GetDevice()->Execute(commandList.get());
    }

    // execute collected geometry pass command list
    if (geometryPassTask != NFE_INVALID_TASK_ID)
    {
        threadPool->WaitForTask(geometryPassTask);
        renderer->GetDevice()->Execute(geometryPassCL.get());
    }

    // execute collected light pass command list
    if (lightsPassTask != NFE_INVALID_TASK_ID)
    {
        threadPool->WaitForTask(lightsPassTask);
        renderer->GetDevice()->Execute(lightsPassCL.get());
    }

    // execute collected debug layer pass command list
    if (debugLayerTask != NFE_INVALID_TASK_ID)
    {
        threadPool->WaitForTask(debugLayerTask);
        renderer->GetDevice()->Execute(debugLayerCL.get());
    }
}

RendererSystem::RendererSystem(SceneManager* scene)
    : mScene(scene)
{
    using namespace std::placeholders;

    EntityManager* entityManager = mScene->GetEntityManager();

    // register entity manager listeners

    mMeshesListener.AddFilter<MeshComponent>();
    mMeshesListener.AddFilter<TransformComponent>();
    mMeshesListener.onCreated = std::bind(&RendererSystem::OnMeshEntityCreated, this, _1);
    mMeshesListener.onRemoved = std::bind(&RendererSystem::OnMeshEntityRemoved, this, _1);
    mMeshesListener.onChanged = std::bind(&RendererSystem::OnMeshEntityChanged, this, _1);
    entityManager->RegisterEntityListener(&mMeshesListener);

    mLightsListener.AddFilter<LightComponent>();
    mLightsListener.AddFilter<TransformComponent>();
    mLightsListener.onCreated = std::bind(&RendererSystem::OnLightEntityCreated, this, _1);
    mLightsListener.onRemoved = std::bind(&RendererSystem::OnLightEntityRemoved, this, _1);
    mLightsListener.onChanged = std::bind(&RendererSystem::OnLightEntityChanged, this, _1);
    entityManager->RegisterEntityListener(&mLightsListener);
}

RendererSystem::~RendererSystem()
{
    EntityManager* entityManager = mScene->GetEntityManager();

    entityManager->UnregisterEntityListener(&mLightsListener);
    entityManager->UnregisterEntityListener(&mMeshesListener);
}

void RendererSystem::RenderLights(const Common::TaskContext& context, RenderingData& data) const
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);
    renderCtx->commandBuffer->Reset();

    LightsRenderer::Get()->Enter(renderCtx);
    LightsRenderer::Get()->SetUp(renderCtx,
                                 data.view->GetRenderTarget(),
                                 data.view->GetGeometryBuffer(),
                                 &data.cameraRenderDesc);

    EnviromentDesc envDesc;
    mScene->GetEnvironment(&envDesc);
    LightsRenderer::Get()->DrawAmbientLight(renderCtx,
                                            renderer->GammaFix(envDesc.ambientLight),
                                            renderer->GammaFix(envDesc.backgroundColor));



    for (const LightEntry* lightEntry : data.visibleOmniLights)
    {
        const TransformComponent* transform = lightEntry->transform;
        const LightComponent* light = lightEntry->light;

        LightsRenderer::Get()->DrawOmniLight(renderCtx, transform->GetPosition(),
                                             light->mOmniLight.radius,
                                             renderer->GammaFix(light->mColor),
                                             light->mShadowMap.get());
    }

    for (const LightEntry* lightEntry : data.visibleSpotLights)
    {
        const TransformComponent* transform = lightEntry->transform;
        const LightComponent* light = lightEntry->light;

        SpotLightProperties prop;
        prop.color = renderer->GammaFix(light->mColor);
        prop.position = transform->GetPosition();
        prop.direction = transform->GetMatrix().r[2];
        prop.farDist.Set(light->mSpotLight.farDist);
        prop.viewProjMatrix = lightEntry->viewMatrix * lightEntry->projMatrix;
        prop.viewProjMatrixInv = MatrixInverse(prop.viewProjMatrix); // TODO move to SpotLightData?
        prop.shadowMapProps = Vector();

        if (light->mShadowMap)
        {
            prop.shadowMapProps = Vector(1.0f / light->mShadowMap->GetSize());
        }

        LightsRenderer::Get()->DrawSpotLight(renderCtx, prop, light->mShadowMap.get(),
                                             light->mLightMapBindingInstance.get());
    }

    // TODO
    /*
    for (const auto& pair : mDirLights)
    {
        const LightEntry& lightEntry = pair.second;
        TransformComponent* transform = lightEntry.transform;
        LightComponent* light = lightEntry.light;

        DirLightProperties prop;
        prop.color = renderer->GammaFix(light->mColor);
        prop.direction = transform->GetMatrix().GetRow(2);
        prop.cascadesCount[0] = light->mDirLight.splits;

        for (int i = 0; i < light->mDirLight.splits; i++)
        {
            // TODO: calculate view and projection matrices
            prop.splitDistance[i] = Vector(); // TODO
            prop.viewProjMatrix[i] = Matrix();
        }

        LightsRenderer::Get()->DrawDirLight(renderCtx, prop, light->mShadowMap.get());
    }
    */

    LightsRenderer::Get()->DrawFog(renderCtx);
    LightsRenderer::Get()->Leave(renderCtx);
    data.lightsPassCL = std::move(renderCtx->commandBuffer->Finish());
}

void RendererSystem::RenderLightsDebug(RenderingData& data, Renderer::RenderContext* ctx) const
{
    uint32 lightDebugColor = 0xFF00FFFF;

    for (const LightEntry* lightEntry : data.visibleOmniLights)
    {
        const TransformComponent* transform = lightEntry->transform;
        const LightComponent* light = lightEntry->light;

        Box box;
        box.min = transform->GetPosition() - VectorSplat(light->mOmniLight.radius);
        box.max = transform->GetPosition() + VectorSplat(light->mOmniLight.radius);
        DebugRenderer::Get()->DrawBox(ctx, box, lightDebugColor);
    }

    for (const LightEntry* lightEntry : data.visibleSpotLights)
    {
        DebugRenderer::Get()->DrawFrustum(ctx, lightEntry->frustum, lightDebugColor);
    }

    // TODO: dir light
}

void RendererSystem::RenderGeometry(RenderContext* ctx, const Math::Frustum& viewFrustum,
                                    const TransformComponent* cameraTransform) const
{
    // draw meshes
    std::vector<MeshEntry> visibleMeshes; //TODO: dynamic allocation per frame should be avoided
    FindVisibleMeshEntities(viewFrustum, visibleMeshes);
    RenderCommandBuffer commandBuffer;

    for (const auto& data : visibleMeshes)
    {
        const Mesh* meshResource = data.mesh->GetMeshResource();

        RenderCommand command;
        command.matrix = data.transform->GetMatrix();
        if (data.body != nullptr)
        {
            command.velocity = data.body->GetVelocity();
            command.angularVelocity = data.body->GetAngularVelocity();
        }
        else
        {
            command.velocity = Vector();
            command.angularVelocity = Vector();
        }
        command.pVB = meshResource->mVB.get();
        command.pIB = meshResource->mIB.get();
        command.distance = VectorLength3(cameraTransform->GetPosition() -
                                         data.transform->GetPosition()).f[0];

        for (const auto& subMesh : meshResource->mSubMeshes)
        {
            // TODO: submesh frustum culling
            command.indexCount = 3 * subMesh.trianglesCount;
            command.startIndex = subMesh.indexOffset;
            command.material = subMesh.material->GetRendererData();
            commandBuffer.PushBack(command);
        }
    }
    visibleMeshes.clear();
    commandBuffer.Sort();

    GeometryRenderer::Get()->Draw(ctx, commandBuffer);
}

void RendererSystem::RenderSpotShadowMap(const Common::TaskContext& context,
                                         const LightEntry* lightData,
                                         RenderingData& data) const
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);
    renderCtx->commandBuffer->Reset();

    ShadowCameraRenderDesc cameraDesc;
    cameraDesc.viewProjMatrix = lightData->viewMatrix * lightData->projMatrix;
    cameraDesc.lightPos = lightData->transform->GetPosition();

    GeometryRenderer::Get()->Enter(renderCtx);
    GeometryRenderer::Get()->SetUpForShadowMap(renderCtx, lightData->light->mShadowMap.get(),
                                               &cameraDesc, 0);
    RenderGeometry(renderCtx, lightData->frustum, lightData->transform);
    GeometryRenderer::Get()->Leave(renderCtx);

    data.shadowPassCLs[context.threadId].push_back(std::move(renderCtx->commandBuffer->Finish()));
};

void RendererSystem::RenderOmniShadowMap(const Common::TaskContext& context,
                                         const LightEntry* lightData,
                                         RenderingData& data) const
{
    // The below vector arrays are front, up and right vectors for each cubemap face camera matrix:

    // Z axis for each cube map face
    static const Vector zVectors[] =
    {
        Vector(1.0f,  0.0f,  0.0f),
        Vector(-1.0f,  0.0f,  0.0f),
        Vector(0.0f,  1.0f,  0.0f),
        Vector(0.0f, -1.0f,  0.0f),
        Vector(0.0f,  0.0f,  1.0f),
        Vector(0.0f,  0.0f, -1.0f),
    };

    // Y axis for each cube map face
    static const Vector yVectors[] =
    {
        Vector(0.0f, 1.0f,  0.0f),
        Vector(0.0f, 1.0f,  0.0f),
        Vector(0.0f, 0.0f, -1.0f),
        Vector(0.0f, 0.0f,  1.0f),
        Vector(0.0f, 1.0f,  0.0f),
        Vector(0.0f, 1.0f,  0.0f),
    };

    // X axis for each cube map face
    static const Vector xVectors[] =
    {
        Vector(0.0f, 0.0f, -1.0f),
        Vector(0.0f, 0.0f, 1.0f),
        Vector(1.0f, 0.0f, 0.0f),
        Vector(1.0f, 0.0f, 0.0f),
        Vector(1.0f, 0.0f, 0.0f),
        Vector(-1.0f, 0.0f, 0.0f),
    };

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    uint32 face = static_cast<uint32>(context.instanceId);
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);
    renderCtx->commandBuffer->Reset();

    // TODO: include "transform" rotation
    Matrix matrix = Matrix(xVectors[face], yVectors[face], zVectors[face],
                           lightData->transform->GetPosition());

    Matrix viewMatrix, projMatrix;
    Frustum frustum;
    SetupPerspective(matrix, 0.01f, lightData->light->mOmniLight.radius, NFE_MATH_PI / 2.0f, 1.0f,
                     viewMatrix, projMatrix, frustum);

    ShadowCameraRenderDesc cameraDesc;
    cameraDesc.viewProjMatrix = viewMatrix * projMatrix;
    cameraDesc.lightPos = lightData->transform->GetPosition();

    GeometryRenderer::Get()->Enter(renderCtx);
    GeometryRenderer::Get()->SetUpForShadowMap(renderCtx, lightData->light->mShadowMap.get(),
                                               &cameraDesc, face);
    RenderGeometry(renderCtx, frustum, lightData->transform);
    GeometryRenderer::Get()->Leave(renderCtx);

    data.shadowPassCLs[context.threadId].push_back(std::move(renderCtx->commandBuffer->Finish()));
};

void RendererSystem::RenderShadowMaps(const Common::TaskContext& context, RenderingData& data)
{
    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    for (LightEntry* lightEntry : data.visibleSpotLights)
    {
        // mark the shadow maps as drawn, continue if it was drawn in another view
        if (lightEntry->shadowDrawn.exchange(true))
            continue;

        if (lightEntry->light->HasShadowMap())
        {
            using namespace std::placeholders;
            threadPool->CreateTask(std::bind(&RendererSystem::RenderSpotShadowMap, this, _1,
                                             lightEntry, std::ref(data)),
                                   1,  // one instance
                                   context.taskId);
        }
    }

    for (LightEntry* lightEntry : data.visibleOmniLights)
    {
        // mark the shadow maps as drawn, continue if it was drawn in another view
        if (lightEntry->shadowDrawn.exchange(true))
            continue;

        if (lightEntry->light->HasShadowMap())
        {
            using namespace std::placeholders;
            threadPool->CreateTask(std::bind(&RendererSystem::RenderOmniShadowMap, this, _1, lightEntry, std::ref(data)),
                                   6,  // six cube faces - six intstances
                                   context.taskId);
        }
    }

    // TODO: dir light
}

void RendererSystem::Render(const Common::TaskContext& context, RenderingData& renderingData)
{
    using namespace std::placeholders;

    EntityManager* entityManager = mScene->GetEntityManager();
    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    /// extract viewing camera information
    EntityID cameraEntity = renderingData.view->GetCameraEntity();
    if (cameraEntity == gInvalidEntityID)
    {
        LOG_WARNING("Invalid camera entity");
        return;
    }

    auto camera = entityManager->GetComponent<CameraComponent>(cameraEntity);
    if (camera == nullptr)
    {
        LOG_WARNING("Camera entity has no camera component");
        return;
    }

    auto cameraTransform = entityManager->GetComponent<TransformComponent>(cameraEntity);
    if (cameraTransform == nullptr)
    {
        LOG_WARNING("Camera entity has no transform component");
        return;
    }

    auto cameraBody = entityManager->GetComponent<BodyComponent>(cameraEntity);
    Vector cameraVelocity;
    Vector cameraAngularVelocity;
    if (cameraBody != nullptr)
    {
        cameraVelocity = cameraBody->GetVelocity();
        cameraAngularVelocity = cameraBody->GetAngularVelocity();
    }

    // set up rendering data structure
    renderingData.cameraComponent = camera;
    renderingData.cameraTransform = cameraTransform;
    renderingData.shadowPassCLs.resize(threadPool->GetThreadsNumber());
    renderingData.cameraRenderDesc =
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

    // frustum culling for lights
    FindVisibleLights(camera->mFrustum, renderingData);

    renderingData.shadowPassTask = threadPool->CreateTask(
        std::bind(&RendererSystem::RenderShadowMaps, this, _1, std::ref(renderingData)));

    auto drawGeometryBufferFunc = [this](const Common::TaskContext& context, RenderingData& data)
    {
        HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
        RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);
        renderCtx->commandBuffer->Reset();

        GeometryRenderer::Get()->Enter(renderCtx);
        GeometryRenderer::Get()->SetUp(renderCtx,
                                       data.view->GetGeometryBuffer(),
                                       &data.cameraRenderDesc);
        RenderGeometry(renderCtx, data.cameraComponent->mFrustum, data.cameraTransform);

        GeometryRenderer::Get()->Leave(renderCtx);
        data.geometryPassCL = renderCtx->commandBuffer->Finish();
    };

    // enqueue geometry pass task
    renderingData.geometryPassTask = threadPool->CreateTask(
        std::bind(drawGeometryBufferFunc, _1, std::ref(renderingData)));

    // enqueue light pass task
    renderingData.lightsPassTask = threadPool->CreateTask(
        std::bind(&RendererSystem::RenderLights, this, _1, std::ref(renderingData)));

    // enqueue debug layer pass task
    // TODO: temporary - add "if" statement when renderer configuration is implemented
    {
        renderingData.debugLayerTask = threadPool->CreateTask(
            std::bind(&RendererSystem::RenderDebugLayer, this, _1, std::ref(renderingData)));
    }
}

void RendererSystem::RenderDebugLayer(const Common::TaskContext& context, RenderingData& data) const
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);

    renderCtx->commandBuffer->Reset();
    DebugRenderer::Get()->Enter(renderCtx);
    DebugRenderer::Get()->SetCamera(renderCtx,
                                    data.cameraComponent->mViewMatrix,
                                    data.cameraComponent->mProjMatrix);
    DebugRenderer::Get()->SetTarget(renderCtx, data.view->GetRenderTarget());

    //draw coordinate system
    {
        DebugRenderer::Get()->DrawLine(renderCtx, Float3(0, 0, 0), Float3(1.0f, 0, 0), 0xFF0000FF);
        DebugRenderer::Get()->DrawLine(renderCtx, Float3(0, 0, 0), Float3(0, 1.0f, 0), 0xFF00FF00);
        DebugRenderer::Get()->DrawLine(renderCtx, Float3(0, 0, 0), Float3(0, 0, 1.0f), 0xFFFF0000);
    }

    // draw entities' coordinate systems
    for (const MeshEntry& entry : mMeshes)
    {
        Float3 start, endX, endY, endZ;
        const float debugSize = 0.2f;

        Matrix matrix = entry.transform->GetMatrix();
        VectorStore(matrix.GetRow(3), &start);
        VectorStore(debugSize * (matrix.GetRow(0)) + matrix.GetRow(3), &endX);
        VectorStore(debugSize * (matrix.GetRow(1)) + matrix.GetRow(3), &endY);
        VectorStore(debugSize * (matrix.GetRow(2)) + matrix.GetRow(3), &endZ);

        DebugRenderer::Get()->DrawLine(renderCtx, start, endX, 0xFF0000FF);
        DebugRenderer::Get()->DrawLine(renderCtx, start, endY, 0xFF00FF00);
        DebugRenderer::Get()->DrawLine(renderCtx, start, endZ, 0xFFFF0000);
    }

    // draw light shapes
    // TODO: temporary - fix "if" statement when renderer configuration is implemented
    if (false)
    {
        RenderLightsDebug(data, renderCtx);
    }

    // draw meshes AABBs
    // TODO: temporary - fix "if" statement when renderer configuration is implemented
    if (false)
    {
        for (const MeshEntry& entry : mMeshes)
        {
            // TODO: change to boxes only
            DebugRenderer::Get()->DrawMesh(renderCtx, entry.mesh->GetMeshResource(), entry.transform->GetMatrix());
        }
    }

    DebugRenderer::Get()->Leave(renderCtx);
    data.debugLayerCL = renderCtx->commandBuffer->Finish();
}

// build list of meshes visible in a frustum
void RendererSystem::FindVisibleMeshEntities(const Frustum& frustum,
                                             std::vector<MeshEntry>& list) const
{
    /*
    mMeshesBVH.Query(frustum, [&](void* userData)
    {
        const EntityID entity = static_cast<EntityID>(reinterpret_cast<size_t>(userData));

        const auto& it = mMeshes.find(entity);

        list.push_back(it->second); // TODO this is slow...
    });
    */
    
    
    for (const MeshEntry& entry : mMeshes)
    {
        if (Intersect(entry.aabb, frustum))
            list.push_back(entry);
    }
}

void RendererSystem::FindVisibleLights(const Math::Frustum& frustum, RenderingData& data)
{
    for (auto& it : mLights)
    {
        TransformComponent* transform = it.second.transform;
        LightComponent* light = it.second.light;

        if (light->mLightType == LightType::Omni)
        {
            Sphere lightSphere;
            lightSphere.origin = transform->GetPosition();
            lightSphere.r = light->mOmniLight.radius;

            if (Intersect(frustum, lightSphere))
                data.visibleOmniLights.push_back(&it.second);
        }
        else if (light->mLightType == LightType::Spot)
        {
            if (Intersect(frustum, it.second.frustum))
                data.visibleSpotLights.push_back(&it.second);
        }
    }
}

void RendererSystem::OnMeshEntityCreated(EntityID entity)
{
    EntityManager* entityManager = mScene->GetEntityManager();

    size_t meshID = mMeshes.size();
    mMeshesEntityMapping[entity] = meshID;

    // setup mesh entry
    MeshEntry entry;
    entry.mesh = entityManager->GetComponent<MeshComponent>(entity);
    entry.transform = entityManager->GetComponent<TransformComponent>(entity);
    entry.body = entityManager->GetComponent<BodyComponent>(entity);
    entry.aabb = entry.mesh->CalculateAABB(entry.transform->GetMatrix());
    entry.bvhNode = mMeshesBVH.Insert(entry.aabb, reinterpret_cast<void*>(meshID));
    mMeshes.push_back(entry);
}

void RendererSystem::OnMeshEntityRemoved(EntityID entity)
{
    const auto& iter = mMeshesEntityMapping.find(entity);
    if (iter == mMeshesEntityMapping.end())
        return;

    size_t id = iter->second;
    mMeshesBVH.Remove(mMeshes[id].bvhNode);
    mMeshesEntityMapping.erase(iter);

    if (mMeshes.size() > 0)
    {
        // move last mesh in place of removed
        mMeshes[id] = mMeshes[mMeshes.size() - 1];
        mMeshesEntityMapping[mMeshes[id].entityID] = id;
        mMeshesBVH.SetUserData(mMeshes[id].bvhNode, reinterpret_cast<void*>(id));
    }
}

void RendererSystem::OnMeshEntityChanged(EntityID entity)
{
    const auto& iter = mMeshesEntityMapping.find(entity);
    if (iter == mMeshesEntityMapping.end())
        return;

    MeshEntry& entry = mMeshes[iter->second];

    // TODO: AABB should be only updated when transform has changed
    // TODO: notification about source component
    entry.aabb = entry.mesh->CalculateAABB(entry.transform->GetMatrix());
    mMeshesBVH.Move(entry.bvhNode, entry.aabb);
}


void RendererSystem::OnLightEntityCreated(EntityID entity)
{
    EntityManager* entityManager = mScene->GetEntityManager();

    LightEntry entry;
    entry.light = entityManager->GetComponent<LightComponent>(entity);
    entry.transform = entityManager->GetComponent<TransformComponent>(entity);
    mLights[entity] = entry;

    OnLightEntityChanged(entity);
}

void RendererSystem::OnLightEntityRemoved(EntityID entity)
{
    mLights.erase(entity);
}

void RendererSystem::OnLightEntityChanged(EntityID entity)
{
    const auto& it = mLights.find(entity);

    if (it == mLights.end())
        return;

    LightEntry& entry = it->second;
    
    if (entry.light->mLightType == LightType::Spot)
    {
        SetupPerspective(entry.transform->GetMatrix(),
                         entry.light->mSpotLight.nearDist,
                         entry.light->mSpotLight.farDist,
                         entry.light->mSpotLight.cutoff, 1.0f,
                         entry.viewMatrix, entry.projMatrix, entry.frustum);
    }
}

void RendererSystem::UpdateCameras()
{
    EntityManager* entityManager = mScene->GetEntityManager();
    mCameras.clear();

    entityManager->ForEach_DEPRECATED<TransformComponent, CameraComponent>(
        [&](EntityID entity,
            TransformComponent* transform,
            CameraComponent* camera)
        {
            BodyComponent* body = entityManager->GetComponent<BodyComponent>(entity);
            mCameras.push_back(std::make_tuple(transform, camera, body));

            Vector velocity;
            Vector angularVelocity;
            if (body)
            {
                velocity = body->GetVelocity();
                angularVelocity = body->GetAngularVelocity();
            }
            camera->Update(transform->GetMatrix(), velocity, angularVelocity, 0.0f);
        }
    );
}

void RendererSystem::UpdateLights()
{
    // TODO: instead of iterating through everything we can keep list of lights,
    // for which shadow map was drawn in previous frame

    // mark all shadow maps as not-drawn
    for (auto& it : mLights)
    {
        it.second.shadowDrawn = false;
    }
}

void RendererSystem::Update(const Common::TaskContext& context, float dt)
{
    context.pool->CreateTask(std::bind(&RendererSystem::UpdateCameras, this), 1, context.taskId);
    context.pool->CreateTask(std::bind(&RendererSystem::UpdateLights, this), 1, context.taskId);
}

} // namespace Scene
} // namespace NFE

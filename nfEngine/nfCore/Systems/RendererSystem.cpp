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
#include "Utils/ConfigVariable.hpp"

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

#include "nfCommon/System/Timer.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/Math/Geometry.hpp"
#include "nfCommon/Math/Sphere.hpp"

namespace NFE {

// debug drawing config vars
namespace {
ConfigVariable<bool> gDrawDebugEnable("renderer/debug/enable", true);
ConfigVariable<bool> gDrawDebugLights("renderer/debug/lights", false);
ConfigVariable<bool> gDrawDebugMeshes("renderer/debug/meshes", false);
} // namespace

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
{}

RenderingData::RenderingData(const RenderingData& other)
{
}

void RenderingData::WaitForRenderingTasks() const
{
    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    // wait for the main renderer task - it spawns the rest of tasks used here...
    if (sceneRenderTask != NFE_INVALID_TASK_ID)
        threadPool->WaitForTask(sceneRenderTask);

    // execute collected shadow command lists
    if (shadowPassTask != NFE_INVALID_TASK_ID)
    {
        threadPool->WaitForTask(shadowPassTask);
    }

    // execute collected geometry pass command list
    if (geometryPassTask != NFE_INVALID_TASK_ID)
    {
        threadPool->WaitForTask(geometryPassTask);
    }

    // execute collected light pass command list
    if (lightsPassTask != NFE_INVALID_TASK_ID)
    {
        threadPool->WaitForTask(lightsPassTask);
    }

    // execute collected debug layer pass command list
    if (debugLayerTask != NFE_INVALID_TASK_ID)
    {
        threadPool->WaitForTask(debugLayerTask);
    }
}

RendererSystem::RendererSystem(SceneManager* scene)
    : mScene(scene)
{
}

void RendererSystem::RenderLights(const Common::TaskContext& context, RenderingData& data) const
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);

    LightsRenderer::Get()->OnEnter(renderCtx->lightsContext.get());
    LightsRenderer::Get()->SetUp(renderCtx->lightsContext.get(),
                                 data.view->GetRenderTarget(),
                                 data.view->GetGeometryBuffer(),
                                 &data.cameraRenderDesc);

    EnviromentDesc envDesc;
    mScene->GetEnvironment(&envDesc);
    LightsRenderer::Get()->DrawAmbientLight(renderCtx->lightsContext.get(),
                                            renderer->GammaFix(envDesc.ambientLight),
                                            renderer->GammaFix(envDesc.backgroundColor));



    for (const uint32 i : data.visibleOmniLights)
    {
        TransformComponent* transform = std::get<0>(mOmniLights[i]);
        LightComponent* light = std::get<1>(mOmniLights[i]);

        LightsRenderer::Get()->DrawOmniLight(renderCtx->lightsContext.get(), transform->GetPosition(),
                                             light->mOmniLight.radius,
                                             renderer->GammaFix(light->mColor),
                                             light->mShadowMap.get());
    }

    for (const uint32 i : data.visibleSpotLights)
    {
        const SpotLightData& lightData = mSpotLightsData[i];
        const TransformComponent* transform = std::get<0>(mSpotLights[i]);
        const LightComponent* light = std::get<1>(mSpotLights[i]);

        SpotLightProperties prop;
        prop.color = renderer->GammaFix(light->mColor);
        prop.position = transform->GetPosition();
        prop.direction = transform->GetMatrix().r[2];
        prop.farDist.Set(light->mSpotLight.farDist);
        prop.viewProjMatrix = lightData.viewMatrix * lightData.projMatrix;
        prop.viewProjMatrixInv = MatrixInverse(prop.viewProjMatrix); // TODO move to SpotLightData?
        prop.shadowMapProps = Vector();

        if (light->mShadowMap)
        {
            prop.shadowMapProps = Vector(1.0f / light->mShadowMap->GetSize());
        }

        LightsRenderer::Get()->DrawSpotLight(renderCtx->lightsContext.get(), prop, light->mShadowMap.get(),
                                             light->mLightMapBindingInstance);
    }

    for (auto lightTuple : mDirLights)
    {
        TransformComponent* transform = std::get<0>(lightTuple);
        LightComponent* light = std::get<1>(lightTuple);

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

        LightsRenderer::Get()->DrawDirLight(renderCtx->lightsContext.get(), prop, light->mShadowMap.get());
    }

    LightsRenderer::Get()->DrawFog(renderCtx->lightsContext.get());
    LightsRenderer::Get()->OnLeave(renderCtx->lightsContext.get());
}

void RendererSystem::RenderLightsDebug(RenderingData& data, Renderer::RenderContext* ctx) const
{
    uint32 lightDebugColor = 0xFF00FFFF;

    for (const uint32 i : data.visibleOmniLights)
    {
        TransformComponent* transform = std::get<0>(mOmniLights[i]);
        LightComponent* light = std::get<1>(mOmniLights[i]);

        Box box;
        box.min = transform->GetPosition() - VectorSplat(light->mOmniLight.radius);
        box.max = transform->GetPosition() + VectorSplat(light->mOmniLight.radius);
        DebugRenderer::Get()->DrawBox(ctx->debugContext.get(), box, lightDebugColor);
    }

    for (const uint32 i : data.visibleSpotLights)
    {
        DebugRenderer::Get()->DrawFrustum(ctx->debugContext.get(), mSpotLightsData[i].frustum, lightDebugColor);
    }

    // TODO: dir light
}

void RendererSystem::RenderGeometry(GeometryRendererContext* ctx, const Math::Frustum& viewFrustum,
                                    const TransformComponent* cameraTransform) const
{
    // draw meshes
    std::vector<MeshEntry> visibleMeshes; //TODO: dynamic allocation per frame should be avoided
    FindVisibleMeshEntities(viewFrustum, visibleMeshes);
    RenderCommandBuffer commandRecorder;

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
        command.pVB = meshResource->mVB;
        command.pIB = meshResource->mIB;
        command.distance = VectorLength3(cameraTransform->GetPosition() -
                                         transform->GetPosition()).f[0];

        for (const auto& subMesh : meshResource->mSubMeshes)
        {
            // TODO: submesh frustum culling
            command.indexCount = 3 * subMesh.trianglesCount;
            command.startIndex = subMesh.indexOffset;
            command.material = subMesh.material->GetRendererData();
            commandRecorder.PushBack(command);
        }
    }
    visibleMeshes.clear();
    commandRecorder.Sort();

    GeometryRenderer::Get()->Draw(ctx, commandRecorder);
}

void RendererSystem::RenderSpotShadowMap(const Common::TaskContext& context,
                                         const SpotLightData& lightData,
                                         const TransformComponent* transform,
                                         const LightComponent* light,
                                         RenderingData& data) const
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);
    GeometryRendererContext* shadowsContext = renderCtx->shadowsContext.get();

    ShadowCameraRenderDesc cameraDesc;
    cameraDesc.viewProjMatrix = lightData.viewMatrix * lightData.projMatrix;
    cameraDesc.lightPos = transform->GetPosition();

    GeometryRenderer::Get()->OnEnter(shadowsContext);
    GeometryRenderer::Get()->SetUpForShadowMap(shadowsContext, light->mShadowMap.get(),
                                               &cameraDesc, 0);
    RenderGeometry(shadowsContext, lightData.frustum, transform);
    GeometryRenderer::Get()->OnLeave(shadowsContext);
};

void RendererSystem::RenderOmniShadowMap(const Common::TaskContext& context,
                                         const TransformComponent* transform,
                                         const LightComponent* light,
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

    // TODO: include "transform" rotation
    Matrix matrix = Matrix(xVectors[face], yVectors[face], zVectors[face],
                           transform->GetPosition());

    Matrix viewMatrix, projMatrix;
    Frustum frustum;
    SetupPerspective(matrix, 0.01f, light->mOmniLight.radius, NFE_MATH_PI / 2.0f, 1.0f,
                     viewMatrix, projMatrix, frustum);

    ShadowCameraRenderDesc cameraDesc;
    cameraDesc.viewProjMatrix = viewMatrix * projMatrix;
    cameraDesc.lightPos = transform->GetPosition();

    GeometryRendererContext* shadowsContext = renderCtx->shadowsContext.get();
    GeometryRenderer::Get()->OnEnter(shadowsContext);
    GeometryRenderer::Get()->SetUpForShadowMap(shadowsContext, light->mShadowMap.get(), &cameraDesc, face);
    RenderGeometry(shadowsContext, frustum, transform);
    GeometryRenderer::Get()->OnLeave(shadowsContext);
};

void RendererSystem::RenderShadowMaps(const Common::TaskContext& context, RenderingData& data)
{
    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    for (const uint32 i : data.visibleSpotLights)
    {
        // mark the shadow maps as drawn, continue if it was drawn in another view
        if (mSpotLightsShadowDrawn[i].exchange(true))
            continue;

        const TransformComponent* transform = std::get<0>(mSpotLights[i]);
        const LightComponent* light = std::get<1>(mSpotLights[i]);
        const SpotLightData& lightData = mSpotLightsData[i];
        if (light->HasShadowMap())
        {
            using namespace std::placeholders;
            threadPool->CreateTask(std::bind(&RendererSystem::RenderSpotShadowMap, this, _1,
                                             std::cref(lightData), transform, light,
                                             std::ref(data)),
                                   1,  // one instance
                                   context.taskId);
        }
    }

    for (const uint32 i : data.visibleOmniLights)
    {
        // mark the shadow maps as drawn, continue if it was drawn in another view
        if (mOmniLightsShadowDrawn[i].exchange(true))
            continue;

        const TransformComponent* transform = std::get<0>(mOmniLights[i]);
        const LightComponent* light = std::get<1>(mOmniLights[i]);
        if (light->HasShadowMap())
        {
            using namespace std::placeholders;
            threadPool->CreateTask(std::bind(&RendererSystem::RenderOmniShadowMap, this, _1,
                                             transform, light, std::ref(data)),
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
        GeometryRendererContext* gbufferContext = renderCtx->geometryContext.get();

        GeometryRenderer::Get()->OnEnter(gbufferContext);
        GeometryRenderer::Get()->SetUp(gbufferContext,
                                       data.view->GetGeometryBuffer(),
                                       &data.cameraRenderDesc);
        RenderGeometry(gbufferContext, data.cameraComponent->mFrustum, data.cameraTransform);
        GeometryRenderer::Get()->OnLeave(gbufferContext);
    };

    // enqueue geometry pass task
    renderingData.geometryPassTask = threadPool->CreateTask(
        std::bind(drawGeometryBufferFunc, _1, std::ref(renderingData)));

    // enqueue light pass task
    renderingData.lightsPassTask = threadPool->CreateTask(
        std::bind(&RendererSystem::RenderLights, this, _1, std::ref(renderingData)));

    // enqueue debug layer pass task
    if (gDrawDebugEnable.Get())
    {
        renderingData.debugLayerTask = threadPool->CreateTask(
            std::bind(&RendererSystem::RenderDebugLayer, this, _1, std::ref(renderingData)));
    }
}

void RendererSystem::RenderDebugLayer(const Common::TaskContext& context, RenderingData& data) const
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);
    DebugRendererContext* debugContext = renderCtx->debugContext.get();

    DebugRenderer::Get()->OnEnter(debugContext);
    DebugRenderer::Get()->SetCamera(debugContext,
                                    data.cameraComponent->mViewMatrix,
                                    data.cameraComponent->mProjMatrix);
    DebugRenderer::Get()->SetTarget(debugContext, data.view->GetRenderTarget());

    //draw coordinate system
    {
        DebugRenderer::Get()->DrawLine(debugContext, Float3(0, 0, 0), Float3(1.0f, 0, 0), 0xFF0000FF);
        DebugRenderer::Get()->DrawLine(debugContext, Float3(0, 0, 0), Float3(0, 1.0f, 0), 0xFF00FF00);
        DebugRenderer::Get()->DrawLine(debugContext, Float3(0, 0, 0), Float3(0, 0, 1.0f), 0xFFFF0000);
    }

    // draw entities' coordinate systems
    for (auto meshTuple : mMeshes)
    {
        TransformComponent* transform = std::get<0>(meshTuple);

        Float3 start, endX, endY, endZ;
        const float debugSize = 0.2f;

        Matrix matrix = transform->GetMatrix();
        VectorStore(matrix.GetRow(3), &start);
        VectorStore(debugSize * (matrix.GetRow(0)) + matrix.GetRow(3), &endX);
        VectorStore(debugSize * (matrix.GetRow(1)) + matrix.GetRow(3), &endY);
        VectorStore(debugSize * (matrix.GetRow(2)) + matrix.GetRow(3), &endZ);

        DebugRenderer::Get()->DrawLine(debugContext, start, endX, 0xFF0000FF);
        DebugRenderer::Get()->DrawLine(debugContext, start, endY, 0xFF00FF00);
        DebugRenderer::Get()->DrawLine(debugContext, start, endZ, 0xFFFF0000);
    }

    // draw light shapes
    if (gDrawDebugLights.Get())
    {
        RenderLightsDebug(data, renderCtx);
    }

    // draw meshes AABBs
    if (gDrawDebugMeshes.Get())
    {
        for (auto meshTuple : mMeshes)
        {
            TransformComponent* transform = std::get<0>(meshTuple);
            MeshComponent* mesh = std::get<1>(meshTuple);

            // TODO: change to boxes only
            DebugRenderer::Get()->DrawMesh(debugContext, mesh->mMesh, transform->GetMatrix());
        }
    }

    DebugRenderer::Get()->OnLeave(debugContext);
}

// build list of meshes visible in a frustum
void RendererSystem::FindVisibleMeshEntities(const Frustum& frustum,
                                             std::vector<MeshEntry>& list) const
{
    for (auto meshTuple : mMeshes)
    {
        MeshComponent* mesh = std::get<1>(meshTuple);

        if (Intersect(mesh->mGlobalAABB, frustum))
            list.push_back(meshTuple);
    }
}

void RendererSystem::FindVisibleLights(const Math::Frustum& frustum, RenderingData& data) const
{
    for (size_t i = 0; i < mOmniLights.size(); ++i)
    {
        TransformComponent* transform = std::get<0>(mOmniLights[i]);
        LightComponent* light = std::get<1>(mOmniLights[i]);

        Sphere lightSphere;
        lightSphere.origin = transform->GetPosition();
        lightSphere.r = light->mOmniLight.radius;

        if (Intersect(frustum, lightSphere))
            data.visibleOmniLights.push_back(static_cast<uint32>(i));
    }

    for (size_t i = 0; i < mSpotLights.size(); ++i)
    {
        const SpotLightData& lightData = mSpotLightsData[i];

        if (Intersect(frustum, lightData.frustum))
            data.visibleSpotLights.push_back(static_cast<uint32>(i));
    }
}

void RendererSystem::UpdateMeshes()
{
    EntityManager* entityManager = mScene->GetEntityManager();
    mMeshes.clear();

    entityManager->ForEach<TransformComponent, MeshComponent>(
        [&](EntityID entity,
            TransformComponent* transform,
            MeshComponent* mesh)
        {
            Mesh* meshResource = mesh->mMesh;
            if (meshResource == nullptr || meshResource->GetState() != ResourceState::Loaded)
                return;

            mesh->CalcAABB(transform->GetMatrix());

            BodyComponent* body = entityManager->GetComponent<BodyComponent>(entity);
            mMeshes.push_back(std::make_tuple(transform, mesh, body));
        }
    );
}

void RendererSystem::UpdateCameras()
{
    EntityManager* entityManager = mScene->GetEntityManager();
    mCameras.clear();

    entityManager->ForEach<TransformComponent, CameraComponent>(
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
        SpotLightData& data = mSpotLightsData[i];

        SetupPerspective(transform->GetMatrix(),
                         light->mSpotLight.nearDist,
                         light->mSpotLight.farDist,
                         light->mSpotLight.cutoff, 1.0f,
                         data.viewMatrix, data.projMatrix, data.frustum);
    }

    // mark all shadow maps as not-drawn
    {
        mSpotLightsShadowDrawn.reset(new std::atomic_bool[mSpotLights.size()]);
        for (size_t i = 0; i < mSpotLights.size(); ++i)
            mSpotLightsShadowDrawn[i] = false;

        mOmniLightsShadowDrawn.reset(new std::atomic_bool[mOmniLights.size()]);
        for (size_t i = 0; i < mOmniLights.size(); ++i)
            mOmniLightsShadowDrawn[i] = false;
    }
}

void RendererSystem::Update(const Common::TaskContext& context, float dt)
{
    context.pool->CreateTask(std::bind(&RendererSystem::UpdateMeshes, this), 1, context.taskId);
    context.pool->CreateTask(std::bind(&RendererSystem::UpdateCameras, this), 1, context.taskId);
    context.pool->CreateTask(std::bind(&RendererSystem::UpdateLights, this), 1, context.taskId);
}

} // namespace Scene
} // namespace NFE

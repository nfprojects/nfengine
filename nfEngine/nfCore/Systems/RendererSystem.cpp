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
#include "../Renderer/LightsRenderer.hpp"
#include "../Renderer/GeometryRenderer.hpp"
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

// TODO: this should be used also by CameraComponent
void CalculateFrustum(const Matrix& matrix,
                      float nearDist, float farDist, float cutoff,
                      Matrix& viewMatrix, Matrix& projMatrix, Frustum& frustum)
{
    // calculate view and projection matrices
    viewMatrix = MatrixLookTo(matrix.GetRow(3), matrix.GetRow(2), matrix.GetRow(1));
    projMatrix = MatrixPerspective(1.0f, cutoff, farDist, nearDist);

    Vector pos = matrix.GetRow(3) & VECTOR_MASK_XYZ;
    float scale = tanf(cutoff / 2.0f);
    Vector xAxis = scale * matrix.GetRow(0);
    Vector yAxis = scale * matrix.GetRow(1);
    Vector zAxis = scale * matrix.GetRow(2);

    frustum.Construct(pos, xAxis, yAxis, zAxis, nearDist, farDist);
}

RendererSystem::RendererSystem(SceneManager* scene)
    : mScene(scene)
{
}

void RendererSystem::RenderLights(RenderingData& data, RenderContext* ctx) const
{
    for (const uint32 i : data.visibleOmniLights)
    {
        TransformComponent* transform = std::get<0>(mOmniLights[i]);
        LightComponent* light = std::get<1>(mOmniLights[i]);

        LightsRenderer::Get()->DrawOmniLight(ctx, transform->GetPosition(),
                                             light->mOmniLight.radius, light->mColor,
                                             light->mShadowMap.get());
    }

    for (const uint32 i : data.visibleSpotLights)
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
        prop.shadowMapProps = Vector();

        if (light->mShadowMap)
        {
            prop.shadowMapProps = Vector(1.0f / light->mShadowMap->GetSize());
        }

        ITexture* lightMap = light->mLightMap ? light->mLightMap->GetRendererTexture() : nullptr;

        LightsRenderer::Get()->DrawSpotLight(ctx, prop, light->mShadowMap.get(), lightMap);
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

        LightsRenderer::Get()->DrawDirLight(ctx, prop, light->mShadowMap.get());
    }
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
        DebugRenderer::Get()->DrawBox(ctx, box, lightDebugColor);
    }

    for (const uint32 i : data.visibleSpotLights)
    {
        DebugRenderer::Get()->DrawFrustum(ctx, mSpotLightsData[i].frustum, lightDebugColor);
    }

    // TODO: dir light
}

void RendererSystem::DrawGeometry(RenderContext* ctx, const Math::Frustum& viewFrustum,
                                  const TransformComponent* cameraTransform) const
{
    // draw meshes
    std::vector<MeshEntry> visibleMeshes; //TODO: dynamic allocation per frame should be avoided
    FindVisibleMeshEntities(viewFrustum, visibleMeshes);
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

    GeometryRenderer::Get()->Draw(ctx, commandBuffer);
}

void RendererSystem::RenderShadowMaps(const Common::TaskContext& context, RenderingData& data)
{
    /*
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* immCtx = renderer->GetImmediateContext();
    for (const uint32 i : data.visibleSpotLights)
    {
        // TODO: atomic
        if (!mSpotLightsShadowDrawn[i])
            mSpotLightsShadowDrawn[i] = true;
        else
            continue;

        const SpotLightData& lightData = mSpotLightsData[i];
        const TransformComponent* transform = std::get<0>(mSpotLights[i]);
        const LightComponent* light = std::get<1>(mSpotLights[i]);

        if (light->HasShadowMap())
        {
            ShadowCameraRenderDesc cameraDesc;
            cameraDesc.viewProjMatrix = lightData.viewMatrix * lightData.projMatrix;
            cameraDesc.lightPos = transform->GetPosition();

            GeometryRenderer::Get()->SetUpForShadowMap(immCtx, light->mShadowMap.get(),
                                                       &cameraDesc, 0);
            DrawGeometry(immCtx, lightData.frustum, transform);
        }
    }
    */

    // Z axis for each cube map face
    static Vector zVectors[] =
    {
        Vector( 1.0f,  0.0f,  0.0f),
        Vector(-1.0f,  0.0f,  0.0f),
        Vector( 0.0f,  1.0f,  0.0f),
        Vector( 0.0f, -1.0f,  0.0f),
        Vector( 0.0f,  0.0f,  1.0f),
        Vector( 0.0f,  0.0f, -1.0f),
    };

    // Y axis for each cube map face
    static Vector yVectors[] =
    {
        Vector(0.0f, 1.0f,  0.0f),
        Vector(0.0f, 1.0f,  0.0f),
        Vector(0.0f, 0.0f, -1.0f),
        Vector(0.0f, 0.0f,  1.0f),
        Vector(0.0f, 1.0f,  0.0f),
        Vector(0.0f, 1.0f,  0.0f),
    };

    auto omniShadowMapRenderingFuc = [this](const Common::TaskContext& context,
                                            const TransformComponent* transform,
                                            const LightComponent* light)
    {
        HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
        uint32 face = static_cast<uint32>(context.instanceId);
        RenderContext* renderContext = renderer->GetDeferredContext(context.threadId);

        // TODO: include "transform" rotation
        Matrix matrix = Matrix(VectorCross3(yVectors[face], zVectors[face]),
                               yVectors[face],
                               zVectors[face],
                               transform->GetPosition());

        Matrix viewMatrix, projMatrix;
        Frustum frustum;
        CalculateFrustum(matrix, 0.01f, light->mOmniLight.radius, NFE_MATH_PI / 2.0f,
                         viewMatrix, projMatrix, frustum);

        ShadowCameraRenderDesc cameraDesc;
        cameraDesc.viewProjMatrix = viewMatrix * projMatrix;
        cameraDesc.lightPos = transform->GetPosition();

        GeometryRenderer::Get()->SetUpForShadowMap(renderContext, light->mShadowMap.get(),
                                                   &cameraDesc, face);
        DrawGeometry(renderContext, frustum, transform);
    };

    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    for (const uint32 i : data.visibleOmniLights)
    {
        // TODO: atomic
        if (!mOmniLightsShadowDrawn[i])
            mOmniLightsShadowDrawn[i] = true;
        else
            continue;

        const TransformComponent* transform = std::get<0>(mOmniLights[i]);
        const LightComponent* light = std::get<1>(mOmniLights[i]);

        if (light->HasShadowMap())
        {
            using namespace std::placeholders;
            threadPool->CreateTask(std::bind(omniShadowMapRenderingFuc, _1, transform, light),
                                   6, context.taskId);
        }
    }

    // TODO: dir light
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

    // TODO: avoid dynamic allocation
    RenderingData renderingData;
    FindVisibleLights(camera->mFrustum, renderingData);

    {
        for (int i = 0; i < 8; ++i)
        {
            RenderContext* context = renderer->GetDeferredContext(i);
            GeometryRenderer::Get()->Enter(context);
        }

        using namespace std::placeholders;
        Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();
        Common::TaskID shadowTask = threadPool->CreateTask(std::bind(&RendererSystem::RenderShadowMaps, this, _1, std::ref(renderingData)));
        threadPool->WaitForTask(shadowTask);

        for (int i = 0; i < 8; ++i)
        {
            RenderContext* context = renderer->GetDeferredContext(i);
            GeometryRenderer::Get()->Leave(context);
            ICommandList* list = context->commandBuffer->Finish();
            // immCtx->commandBuffer->Execute(list);
            delete list;
        }
    }

    GeometryRenderer::Get()->Enter(immCtx);
    GeometryRenderer::Get()->SetUp(immCtx, view->GetGeometryBuffer(), &cameraRenderDesc);
    DrawGeometry(immCtx, camera->mFrustum, cameraTransform);
    GeometryRenderer::Get()->Leave(immCtx);

    // LIGHTS RENDERING ===========================================================================
    {
        LightsRenderer::Get()->Enter(immCtx);
        LightsRenderer::Get()->SetUp(immCtx, view->GetRenderTarget(), view->GetGeometryBuffer(),
                                     &cameraRenderDesc);
        LightsRenderer::Get()->DrawAmbientLight(immCtx,
                                                Vector(0.4f, 0.5f, 0.6f),
                                                Vector(0.2f, 0.25f, 0.3f));
        RenderLights(renderingData, immCtx);

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
            RenderLightsDebug(renderingData, immCtx);
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
void RendererSystem::FindVisibleMeshEntities(const Frustum& frustum,
                                             std::vector<MeshEntry>& list) const
{
    for (auto meshTuple : mActiveMeshEntities)
    {
        MeshComponent* mesh = std::get<1>(meshTuple);

        if (Intersect(mesh->mGlobalAABB, frustum))
            list.push_back(meshTuple);
    }
}

void RendererSystem::FindVisibleLights(const Math::Frustum & frustum, RenderingData& data) const
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

        CalculateFrustum(transform->GetMatrix(),
                         light->mSpotLight.nearDist,
                         light->mSpotLight.farDist,
                         light->mSpotLight.cutoff,
                         data.viewMatrix, data.projMatrix, data.frustum);
    }

    // mark all shadow maps as not-drawn
    mSpotLightsShadowDrawn.clear();
    mSpotLightsShadowDrawn.resize(mSpotLights.size(), false);
    mOmniLightsShadowDrawn.clear();
    mOmniLightsShadowDrawn.resize(mOmniLights.size(), false);
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

    FindActiveMeshEntities();

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

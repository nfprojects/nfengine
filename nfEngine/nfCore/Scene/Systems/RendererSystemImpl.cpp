/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of RenderSystem class.
 */

#include "PCH.hpp"
#include "RendererSystemImpl.hpp"

#include "Engine.hpp"
#include "Utils/ConfigVariable.hpp"

// TODO remove ASAP {
#include "Scene/Entity.hpp"
#include "Scene/Components/ComponentCamera.hpp"
#include "Scene/Components/ComponentBody.hpp"
#include "Resources/Mesh.hpp"
#include "Resources/Material.hpp"
// }

#include "Renderer/HighLevelRenderer.hpp"
#include "Renderer/LightsRenderer.hpp"
#include "Renderer/GeometryRenderer.hpp"
#include "Renderer/DebugRenderer.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/View.hpp"

#include "nfCommon/System/Timer.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/System/Assertion.hpp"
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
    , shadowPassTask(NFE_INVALID_TASK_ID)
    , geometryPassTask(NFE_INVALID_TASK_ID)
    , lightsPassTask(NFE_INVALID_TASK_ID)
    , debugLayerTask(NFE_INVALID_TASK_ID)
{
}

RenderingData::RenderingData(const RenderingData& other)
{
}

RendererSystem::RendererSystem(SceneManager* scene)
    : IRendererSystem(scene)
{
}

RendererSystem::~RendererSystem()
{
}

void RendererSystem::SetEnvironment(const EnvironmentDesc& desc)
{
    mEnvironment = desc;
}

void RendererSystem::GetEnvironment(EnvironmentDesc& desc) const
{
    desc = mEnvironment;
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

    LightsRenderer::Get()->DrawAmbientLight(renderCtx->lightsContext.get(),
                                            renderer->GammaFix(mEnvironment.ambientLight),
                                            renderer->GammaFix(mEnvironment.backgroundColor));

    for (const LightProxy* lightProxy : data.visibleOmniLights)
    {
        const Vector& position = lightProxy->desc.transform[3];

        LightsRenderer::Get()->DrawOmniLight(renderCtx->lightsContext.get(), position,
                                             lightProxy->desc.omni.radius,
                                             lightProxy->desc.color,
                                             lightProxy->shadowMap.get());
    }

    for (const LightProxy* lightProxy : data.visibleSpotLights)
    {
        const LightProxyDesc& desc = lightProxy->desc;

        SpotLightProperties prop;
        prop.color = desc.color;
        prop.position = desc.transform.GetRow(3);
        prop.direction = desc.transform.GetRow(2);
        prop.farDist.Set(desc.spot.farDistance);
        prop.viewProjMatrix = lightProxy->viewMatrix * lightProxy->projMatrix;
        prop.viewProjMatrixInv = prop.viewProjMatrix.Inverted(); // TODO move to SpotLightData?
        prop.shadowMapProps = Vector();

        if (lightProxy->shadowMap)
        {
            prop.shadowMapProps = Vector(1.0f / lightProxy->shadowMap->GetSize());
        }

        LightsRenderer::Get()->DrawSpotLight(renderCtx->lightsContext.get(), prop, lightProxy->shadowMap.get(),
                                             lightProxy->lightMapBindingInstance);
    }

    // TODO
    /*
    for (const auto& pair : mDirLights)
    {
        const LightProxy& lightProxy = pair.second;
        TransformComponent* transform = lightProxy.transform;
        LightComponent* light = lightProxy.light;

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
    */

    LightsRenderer::Get()->DrawFog(renderCtx->lightsContext.get());
    LightsRenderer::Get()->OnLeave(renderCtx->lightsContext.get());
}

void RendererSystem::RenderLightsDebug(RenderingData& data, Renderer::RenderContext* ctx) const
{
    uint32 lightDebugColor = 0xFF00FFFF;

    for (const LightProxy* lightProxy : data.visibleOmniLights)
    {
        const Box box(lightProxy->desc.transform.GetRow(3), lightProxy->desc.omni.radius);
        DebugRenderer::Get()->DrawBox(ctx->debugContext.get(), box, lightDebugColor);
    }

    for (const LightProxy* lightProxy : data.visibleSpotLights)
    {
        DebugRenderer::Get()->DrawFrustum(ctx->debugContext.get(), lightProxy->frustum, lightDebugColor);
    }

    // TODO: dir light
}

void RendererSystem::RenderGeometry(GeometryRendererContext* ctx,
                                    const Frustum& viewFrustum, const Vector& cameraPosition) const
{
    // draw meshes
    std::vector<MeshProxy*> visibleMeshes; //TODO: dynamic allocation per frame should be avoided
    FindVisibleMeshes(viewFrustum, visibleMeshes);
    if (visibleMeshes.empty())
    {
        // nothing to draw
        return;
    }

    RenderCommandBuffer commandBuffer;
    for (const MeshProxy* meshProxy : visibleMeshes)
    {
        const MeshProxyDesc& desc = meshProxy->desc;
        Mesh* meshResource = desc.mesh;
        NFE_ASSERT(meshResource, "Empty mesh resource used in mesh proxy");

        RenderCommand command;
        command.matrix = desc.transform;
        command.velocity = desc.velocity;
        command.angularVelocity = desc.angularVelocity;
        command.vertexBuffer = meshResource->GetVertexBuffer();
        command.indexBuffer = meshResource->GetIndexBuffer();
        command.distance = (cameraPosition - desc.transform.GetRow(3)).Length3();

        // TODO submeshes information should be in MeshProxy
        // renderer should not know about resources
        for (const auto& subMesh : meshResource->GetSubMeshes())
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

void RendererSystem::RenderSpotShadowMap(const Common::TaskContext& context, const LightProxy* lightProxy, RenderingData& data) const
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);
    GeometryRendererContext* shadowsContext = renderCtx->shadowsContext.get();

    ShadowCameraRenderDesc cameraDesc;
    cameraDesc.viewProjMatrix = lightProxy->viewMatrix * lightProxy->projMatrix;
    cameraDesc.lightPos = lightProxy->desc.transform.GetRow(3);

    GeometryRenderer::Get()->OnEnter(shadowsContext);
    GeometryRenderer::Get()->SetUpForShadowMap(shadowsContext, lightProxy->shadowMap.get(), &cameraDesc, 0);
    RenderGeometry(shadowsContext, lightProxy->frustum, cameraDesc.lightPos);
    GeometryRenderer::Get()->OnLeave(shadowsContext);
};

void RendererSystem::RenderOmniShadowMap(const Common::TaskContext& context, const LightProxy* lightProxy, RenderingData& data) const
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
    const Vector& position = lightProxy->desc.transform.GetRow(3);
    Matrix matrix = Matrix(xVectors[face], yVectors[face], zVectors[face], position);

    // calculate frustum, view and projection matrices
    const float nearDistance = 0.01f; // TODO this should be adjustable
    const float farDistance = lightProxy->desc.omni.radius;
    const float cutoff = NFE_MATH_PI / 2.0f;
    const Frustum frustum = Frustum::ConstructForPerspective(matrix, nearDistance, farDistance, cutoff, 1.0f);
    const Matrix viewMatrix = Matrix::MakeLookTo(matrix.GetRow(3), matrix.GetRow(2), matrix.GetRow(1));
    const Matrix projMatrix = Matrix::MakePerspective(1.0f, cutoff, farDistance, nearDistance);
    // TODO write separate versions of the functions above that have cutoff value hardcoded to 90 degrees
    // (to avoid costly trigonometric operations)

    ShadowCameraRenderDesc cameraDesc;
    cameraDesc.viewProjMatrix = viewMatrix * projMatrix;
    cameraDesc.lightPos = position;


    GeometryRendererContext* shadowsContext = renderCtx->shadowsContext.get();
    GeometryRenderer::Get()->OnEnter(shadowsContext);
    GeometryRenderer::Get()->SetUpForShadowMap(shadowsContext, lightProxy->shadowMap.get(), &cameraDesc, face);
    RenderGeometry(shadowsContext, frustum, position);
    GeometryRenderer::Get()->OnLeave(shadowsContext);
};

void RendererSystem::RenderShadowMaps(const Common::TaskContext& context, RenderingData& data)
{
    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    for (LightProxy* lightProxy : data.visibleSpotLights)
    {
        // mark the shadow maps as drawn, continue if it was drawn in another view
        if (lightProxy->shadowDrawn.exchange(true))
            continue;

        if (lightProxy->shadowMap)
        {
            const auto func = [this, &data, lightProxy](const Common::TaskContext& ctx) { RenderSpotShadowMap(ctx, lightProxy, data); };
            threadPool->CreateTask(func, 1, context.taskId);
        }
    }

    for (LightProxy* lightProxy : data.visibleOmniLights)
    {
        // mark the shadow maps as drawn, continue if it was drawn in another view
        if (lightProxy->shadowDrawn.exchange(true))
            continue;

        if (lightProxy->shadowMap)
        {
            const auto func = [this, &data, lightProxy](const Common::TaskContext& ctx) { RenderOmniShadowMap(ctx, lightProxy, data); };
            threadPool->CreateTask(func, 6, context.taskId);
        }
    }

    // TODO: dir light
}

bool RendererSystem::Render(const Common::TaskContext& context, const Renderer::View* view)
{
    using namespace std::placeholders;

    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    // TODO camera descriptor should be passed in a parameter
    // extract viewing camera information
    Entity* cameraEntity = view->GetCameraEntity();
    if (!cameraEntity)
    {
        LOG_WARNING("Invalid camera entity");
        return false;
    }

    const CameraComponent* camera = cameraEntity->GetComponent<CameraComponent>();
    if (camera == nullptr)
    {
        LOG_WARNING("Camera entity has no camera component");
        return false;
    }

    const BodyComponent* cameraBody = cameraEntity->GetComponent<BodyComponent>();
    Vector cameraVelocity;
    Vector cameraAngularVelocity;
    if (cameraBody)
    {
        cameraVelocity = cameraBody->GetVelocity();
        cameraAngularVelocity = cameraBody->GetAngularVelocity();
    }

    // TODO orthogonal camera support
    PerspectiveProjectionDesc perspectiveDesc;
    camera->GetPerspective(&perspectiveDesc);

    RenderingDataPtr renderingData(new RenderingData);

    // set up rendering data structure
    renderingData->view = view;
    renderingData->cameraRenderDesc =
    {
        cameraEntity->GetGlobalTransform().ToMatrix(),
        camera->GetViewMatrix(),
        camera->GetProjectionMatrix(),
        camera->GetSecondaryProjMatrix(),
        cameraVelocity,
        cameraAngularVelocity,
        camera->GetScreenScale(),
        perspectiveDesc.FoV,
    };

    // frustum culling for lights
    FindVisibleLights(camera->GetFrustum(), *renderingData);

    // enqueue shadow maps rendering
    {
        auto func = [this, renderingData](const Common::TaskContext& context)
        {
            RenderShadowMaps(context, *renderingData);
        };
        renderingData->geometryPassTask = threadPool->CreateTask(func, 1, context.taskId);
    }

    // enqueue geometry pass task
    {
        auto func = [this, camera, cameraEntity, renderingData](const Common::TaskContext& context)
        {
            HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
            RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);
            GeometryRendererContext* gbufferContext = renderCtx->geometryContext.get();

            GeometryRenderer::Get()->OnEnter(gbufferContext);
            GeometryRenderer::Get()->SetUp(gbufferContext,
                                           renderingData->view->GetGeometryBuffer(),
                                           &renderingData->cameraRenderDesc);
            RenderGeometry(gbufferContext, camera->GetFrustum(), cameraEntity->GetGlobalPosition());
            GeometryRenderer::Get()->OnLeave(gbufferContext);
        };
        renderingData->geometryPassTask = threadPool->CreateTask(func, 1, context.taskId);
    }

    // enqueue light pass task
    {
        auto func = [this, renderingData](const Common::TaskContext& context)
        {
            RenderLights(context, *renderingData);
        };
        renderingData->geometryPassTask = threadPool->CreateTask(func, 1, context.taskId);
    }

    // enqueue debug layer pass task
    if (gDrawDebugEnable.Get())
    {
        auto func = [this, renderingData](const Common::TaskContext& context)
        {
            RenderDebugLayer(context, *renderingData);
        };
        renderingData->geometryPassTask = threadPool->CreateTask(func, 1, context.taskId);
    }

    return true;
}

void RendererSystem::RenderDebugLayer(const Common::TaskContext& context, RenderingData& data) const
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);
    DebugRendererContext* debugContext = renderCtx->debugContext.get();

    DebugRenderer::Get()->OnEnter(debugContext);
    DebugRenderer::Get()->SetCamera(debugContext,
                                    data.cameraRenderDesc.viewMatrix,
                                    data.cameraRenderDesc.projMatrix);
    DebugRenderer::Get()->SetTarget(debugContext, data.view->GetRenderTarget());

    //draw coordinate system
    {
        DebugRenderer::Get()->DrawLine(debugContext, Float3(0, 0, 0), Float3(1.0f, 0, 0), 0xFF0000FF);
        DebugRenderer::Get()->DrawLine(debugContext, Float3(0, 0, 0), Float3(0, 1.0f, 0), 0xFF00FF00);
        DebugRenderer::Get()->DrawLine(debugContext, Float3(0, 0, 0), Float3(0, 0, 1.0f), 0xFFFF0000);
    }

    // draw light shapes
    if (gDrawDebugLights.Get())
    {
        RenderLightsDebug(data, renderCtx);
    }

    DebugRenderer::Get()->OnLeave(debugContext);
}

void RendererSystem::FindVisibleMeshes(const Frustum& frustum, std::vector<MeshProxy*>& list) const
{
    // TODO optimization: BVH query

    mMeshProxies.Iterate([&frustum, &list](MeshProxy& proxy)
    {
        if (Intersect(proxy.globalBox, frustum))
        {
            list.push_back(&proxy);
        }
    });
}

void RendererSystem::FindVisibleLights(const Frustum& frustum, RenderingData& data) const
{
    // TODO optimization: BVH query

    mLightProxies.Iterate([&frustum, &data](LightProxy& proxy)
    {
        if (proxy.desc.type == LightProxyType::Omni)
        {
            Sphere lightSphere;
            lightSphere.origin = proxy.desc.transform.GetRow(3);
            lightSphere.r = proxy.desc.omni.radius;

            if (Intersect(frustum, lightSphere))
            {
                data.visibleOmniLights.push_back(&proxy);
            }
        }
        else if (proxy.desc.type == LightProxyType::Spot)
        {
            if (Intersect(frustum, proxy.frustum))
            {
                data.visibleSpotLights.push_back(&proxy);
            }
        }
    });
}

RenderProxyID RendererSystem::CreateMeshProxy(const MeshProxyDesc& desc)
{
    MeshProxy proxy;
    proxy.desc = desc;
    proxy.globalBox = desc.mesh->GetGlobalAABB(desc.transform);

    return mMeshProxies.Add(proxy);
}

RenderProxyID RendererSystem::CreateLightProxy(const LightProxyDesc& desc)
{
    LightProxy proxy;
    proxy.desc = desc;

    return mLightProxies.Add(std::move(proxy));
}

bool RendererSystem::UpdateMeshProxy(const RenderProxyID proxyID, const MeshProxyDesc& desc)
{
    NFE_ASSERT(mMeshProxies.Has(proxyID), "Proxy does not exist");

    MeshProxy& proxy = mMeshProxies[proxyID];
    proxy.desc = desc;
    proxy.globalBox = desc.mesh->GetGlobalAABB(desc.transform);

    return true;
}

bool RendererSystem::UpdateLightProxy(const RenderProxyID proxyID, const LightProxyDesc& data)
{
    return false;
}

bool RendererSystem::DeleteMeshProxy(const RenderProxyID proxyID)
{
    return false;
}

bool RendererSystem::DeleteLightProxy(const RenderProxyID proxyID)
{
    return false;
}

void RendererSystem::UpdateLights()
{
    // TODO optimization:
    // instead of iterating through every light, we can keep list of lights
    // for which shadow map was drawn in previous frame

    // TODO optimization:
    // detect lights where nothing in the shadowmap has changed (no mesh has been moved, created, removed
    // and light properties are the same) - we can reuse shadow map from previous frame in this scenario

    // mark all shadow maps as not-drawn
    mLightProxies.Iterate([](LightProxy& proxy)
    {
        proxy.shadowDrawn = false;
    });
}

void RendererSystem::Update(float dt)
{
    UpdateLights();
}

} // namespace Scene
} // namespace NFE

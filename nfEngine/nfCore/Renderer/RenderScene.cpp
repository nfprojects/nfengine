/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of RenderScene class.
 */

#include "PCH.hpp"
#include "RenderScene.hpp"
#include "HighLevelRenderer.hpp"
#include "LightsRenderer.hpp"
#include "GeometryRenderer.hpp"
#include "DebugRenderer.hpp"
#include "RendererContext.hpp"
#include "View.hpp"

#include "Engine.hpp"
#include "Utils/ConfigVariable.hpp"

// TODO remove ASAP {
#include "Scene/Entity.hpp"
#include "Scene/Components/ComponentCamera.hpp"
#include "Scene/Components/ComponentBody.hpp"
#include "Resources/Mesh.hpp"
#include "Resources/Material.hpp"
// }

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

namespace Renderer {

using namespace Math;
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

RenderScene::RenderScene()
{
}

RenderScene::~RenderScene()
{
}

void RenderScene::SetEnvironment(const EnvironmentDesc& desc)
{
    mEnvironment = desc;
}

void RenderScene::GetEnvironment(EnvironmentDesc& desc) const
{
    desc = mEnvironment;
}

void RenderScene::RenderLights(const Common::TaskContext& context, RenderingData& data) const
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);

    LightsRenderer::Get()->OnEnter(renderCtx->lightsContext.Get());
    LightsRenderer::Get()->SetUp(renderCtx->lightsContext.Get(),
                                 data.view->GetRenderTarget(),
                                 data.view->GetGeometryBuffer(),
                                 &data.cameraRenderDesc);

    LightsRenderer::Get()->DrawAmbientLight(renderCtx->lightsContext.Get(),
                                            renderer->GammaFix(mEnvironment.ambientLight),
                                            renderer->GammaFix(mEnvironment.backgroundColor));

    for (const LightProxy* lightProxy : data.visibleOmniLights)
    {
        const Vector4& position = lightProxy->desc.transform[3];

        LightsRenderer::Get()->DrawOmniLight(renderCtx->lightsContext.Get(), position,
                                             lightProxy->desc.omni.radius,
                                             lightProxy->desc.color,
                                             lightProxy->shadowMap.Get());
    }

    for (const LightProxy* lightProxy : data.visibleSpotLights)
    {
        const LightProxyDesc& desc = lightProxy->desc;

        SpotLightProperties prop;
        prop.color = desc.color;
        prop.position = desc.transform[3];
        prop.direction = desc.transform[2];
        prop.farDist = Vector4(desc.spot.farDistance);
        prop.viewProjMatrix = lightProxy->viewMatrix * lightProxy->projMatrix;
        prop.viewProjMatrixInv = prop.viewProjMatrix.Inverted(); // TODO move to SpotLightData?
        prop.shadowMapProps = Vector4::Zero();

        if (lightProxy->shadowMap)
        {
            prop.shadowMapProps = Vector4(1.0f / lightProxy->shadowMap->GetSize());
        }

        LightsRenderer::Get()->DrawSpotLight(renderCtx->lightsContext.Get(), prop, lightProxy->shadowMap.Get(),
                                             lightProxy->lightMapBindingInstance);
    }

    // TODO: dir light

    LightsRenderer::Get()->DrawFog(renderCtx->lightsContext.Get());
    LightsRenderer::Get()->OnLeave(renderCtx->lightsContext.Get());
}

void RenderScene::RenderLightsDebug(RenderingData& data, Renderer::RenderContext* ctx) const
{
    uint32 lightDebugColor = 0xFF00FFFF;

    for (const LightProxy* lightProxy : data.visibleOmniLights)
    {
        const Box box(lightProxy->desc.transform.GetRow(3), lightProxy->desc.omni.radius);
        DebugRenderer::Get()->DrawBox(ctx->debugContext.Get(), box, lightDebugColor);
    }

    for (const LightProxy* lightProxy : data.visibleSpotLights)
    {
        DebugRenderer::Get()->DrawFrustum(ctx->debugContext.Get(), lightProxy->frustum, lightDebugColor);
    }

    // TODO: dir light
}

void RenderScene::RenderGeometry(GeometryRendererContext* ctx,
                                    const Frustum& viewFrustum, const Vector4& cameraPosition) const
{
    // draw meshes
    Common::DynArray<MeshProxy*> visibleMeshes; //TODO: dynamic allocation per frame should be avoided
    FindVisibleMeshes(viewFrustum, visibleMeshes);
    if (visibleMeshes.Empty())
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
    visibleMeshes.Clear();
    commandBuffer.Sort();

    GeometryRenderer::Get()->Draw(ctx, commandBuffer);
}

void RenderScene::RenderSpotShadowMap(const Common::TaskContext& context, const LightProxy* lightProxy, RenderingData& data) const
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);
    GeometryRendererContext* shadowsContext = renderCtx->shadowsContext.Get();

    ShadowCameraRenderDesc cameraDesc;
    cameraDesc.viewProjMatrix = lightProxy->viewMatrix * lightProxy->projMatrix;
    cameraDesc.lightPos = lightProxy->desc.transform.GetRow(3);

    GeometryRenderer::Get()->OnEnter(shadowsContext);
    GeometryRenderer::Get()->SetUpForShadowMap(shadowsContext, lightProxy->shadowMap.Get(), &cameraDesc, 0);
    RenderGeometry(shadowsContext, lightProxy->frustum, cameraDesc.lightPos);
    GeometryRenderer::Get()->OnLeave(shadowsContext);
};

void RenderScene::RenderOmniShadowMap(const Common::TaskContext& context, const LightProxy* lightProxy, RenderingData& data, uint32 faceID) const
{
    // The below vector arrays are front, up and right vectors for each cubemap face camera matrix:

    // Z axis for each cube map face
    static const Vector4 zVectors[] =
    {
        Vector4(1.0f,  0.0f,  0.0f),
        Vector4(-1.0f,  0.0f,  0.0f),
        Vector4(0.0f,  1.0f,  0.0f),
        Vector4(0.0f, -1.0f,  0.0f),
        Vector4(0.0f,  0.0f,  1.0f),
        Vector4(0.0f,  0.0f, -1.0f),
    };

    // Y axis for each cube map face
    static const Vector4 yVectors[] =
    {
        Vector4(0.0f, 1.0f,  0.0f),
        Vector4(0.0f, 1.0f,  0.0f),
        Vector4(0.0f, 0.0f, -1.0f),
        Vector4(0.0f, 0.0f,  1.0f),
        Vector4(0.0f, 1.0f,  0.0f),
        Vector4(0.0f, 1.0f,  0.0f),
    };

    // X axis for each cube map face
    static const Vector4 xVectors[] =
    {
        Vector4(0.0f, 0.0f, -1.0f),
        Vector4(0.0f, 0.0f, 1.0f),
        Vector4(1.0f, 0.0f, 0.0f),
        Vector4(1.0f, 0.0f, 0.0f),
        Vector4(1.0f, 0.0f, 0.0f),
        Vector4(-1.0f, 0.0f, 0.0f),
    };

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);

    // TODO: include "transform" rotation
    const Vector4& position = lightProxy->desc.transform.GetRow(3);
    Matrix4 matrix = Matrix4(xVectors[faceID], yVectors[faceID], zVectors[faceID], position);

    // calculate frustum, view and projection matrices
    const float nearDistance = 0.01f; // TODO this should be adjustable
    const float farDistance = lightProxy->desc.omni.radius;
    const float cutoff = Constants::pi<float> / 2.0f;
    const Frustum frustum = Frustum::ConstructForPerspective(matrix, nearDistance, farDistance, cutoff, 1.0f);
    const Matrix4 viewMatrix = Matrix4::MakeLookTo(matrix.GetRow(3), matrix.GetRow(2), matrix.GetRow(1));
    const Matrix4 projMatrix = Matrix4::MakePerspective(1.0f, cutoff, farDistance, nearDistance);
    // TODO write separate versions of the functions above that have cutoff value hardcoded to 90 degrees
    // (to avoid costly trigonometric operations)

    ShadowCameraRenderDesc cameraDesc;
    cameraDesc.viewProjMatrix = viewMatrix * projMatrix;
    cameraDesc.lightPos = position;


    GeometryRendererContext* shadowsContext = renderCtx->shadowsContext.Get();
    GeometryRenderer::Get()->OnEnter(shadowsContext);
    GeometryRenderer::Get()->SetUpForShadowMap(shadowsContext, lightProxy->shadowMap.Get(), &cameraDesc, faceID);
    RenderGeometry(shadowsContext, frustum, position);
    GeometryRenderer::Get()->OnLeave(shadowsContext);
};

void RenderScene::RenderShadowMaps(const Common::TaskContext& context, RenderingData& data)
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
            for (uint32 i = 0; i < 6; ++i)
            {
                const auto func = [this, &data, lightProxy, i](const Common::TaskContext& ctx)
                {
                    RenderOmniShadowMap(ctx, lightProxy, data, i);
                };
                threadPool->CreateTask(func, context.taskId);
            }
        }
    }

    // TODO: dir light
}

bool RenderScene::Render(const Common::TaskContext& context, const Renderer::View* view)
{
    Common::ThreadPool* threadPool = Engine::GetInstance()->GetThreadPool();

    // TODO camera descriptor should be passed in a parameter
    // extract viewing camera information
    auto* cameraEntity = view->GetCameraEntity();
    if (!cameraEntity)
    {
        NFE_LOG_WARNING("Invalid camera entity");
        return false;
    }

    const auto* camera = cameraEntity->GetComponent<Scene::CameraComponent>();
    if (camera == nullptr)
    {
        NFE_LOG_WARNING("Camera entity has no camera component");
        return false;
    }

    const auto* cameraBody = cameraEntity->GetComponent<Scene::BodyComponent>();
    Vector4 cameraVelocity;
    Vector4 cameraAngularVelocity;
    if (cameraBody)
    {
        cameraVelocity = cameraBody->GetVelocity();
        cameraAngularVelocity = cameraBody->GetAngularVelocity();
    }

    // TODO orthogonal camera support
    Scene::PerspectiveProjectionDesc perspectiveDesc;
    camera->GetPerspective(&perspectiveDesc);

    RenderingDataPtr renderingData = Common::MakeSharedPtr<RenderingData>();

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
        renderingData->geometryPassTask = threadPool->CreateTask(func, context.taskId);
    }

    // enqueue geometry pass task
    {
        auto func = [this, camera, cameraEntity, renderingData](const Common::TaskContext& context)
        {
            HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
            RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);
            GeometryRendererContext* gbufferContext = renderCtx->geometryContext.Get();

            GeometryRenderer::Get()->OnEnter(gbufferContext);
            GeometryRenderer::Get()->SetUp(gbufferContext,
                                           renderingData->view->GetGeometryBuffer(),
                                           &renderingData->cameraRenderDesc);
            RenderGeometry(gbufferContext, camera->GetFrustum(), cameraEntity->GetGlobalPosition());
            GeometryRenderer::Get()->OnLeave(gbufferContext);
        };
        renderingData->geometryPassTask = threadPool->CreateTask(func, context.taskId);
    }

    // enqueue light pass task
    {
        auto func = [this, renderingData](const Common::TaskContext& context)
        {
            RenderLights(context, *renderingData);
        };
        renderingData->geometryPassTask = threadPool->CreateTask(func, context.taskId);
    }

    // enqueue debug layer pass task
    if (gDrawDebugEnable.Get())
    {
        auto func = [this, renderingData](const Common::TaskContext& context)
        {
            RenderDebugLayer(context, *renderingData);
        };
        renderingData->geometryPassTask = threadPool->CreateTask(func, context.taskId);
    }

    return true;
}

void RenderScene::RenderDebugLayer(const Common::TaskContext& context, RenderingData& data) const
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    RenderContext* renderCtx = renderer->GetDeferredContext(context.threadId);
    DebugRendererContext* debugContext = renderCtx->debugContext.Get();

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

void RenderScene::FindVisibleMeshes(const Frustum& frustum, Common::DynArray<MeshProxy*>& list) const
{
    // TODO optimization: BVH query

    mMeshProxies.Iterate([&frustum, &list](MeshProxy& proxy)
    {
        if (Intersect(proxy.globalBox, frustum))
        {
            list.PushBack(&proxy);
        }
    });
}

void RenderScene::FindVisibleLights(const Frustum& frustum, RenderingData& data) const
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
                data.visibleOmniLights.PushBack(&proxy);
            }
        }
        else if (proxy.desc.type == LightProxyType::Spot)
        {
            if (Intersect(frustum, proxy.frustum))
            {
                data.visibleSpotLights.PushBack(&proxy);
            }
        }
    });
}

ProxyID RenderScene::CreateMeshProxy(const MeshProxyDesc& desc)
{
    MeshProxy proxy;
    proxy.desc = desc;
    proxy.globalBox = desc.transform.TransformBox(desc.mesh->GetLocalBox());

    return mMeshProxies.Add(proxy);
}

ProxyID RenderScene::CreateLightProxy(const LightProxyDesc& desc)
{
    LightProxy proxy;
    proxy.desc = desc;

    // create shadow map if enabled
    if (desc.shadowMapSize)
    {
        proxy.shadowMap = Common::MakeUniquePtr<ShadowMap>();

        switch (desc.type)
        {
        case LightProxyType::Omni:
            proxy.shadowMap->Resize(desc.shadowMapSize, ShadowMap::Type::Cube);
            break;
        case LightProxyType::Spot:
            proxy.shadowMap->Resize(desc.shadowMapSize, ShadowMap::Type::Flat);
            break;
        // TODO directional light
        default:
            NFE_LOG_ERROR("Invalid light type");
            return false;
        }
    }

    return mLightProxies.Add(std::move(proxy));
}

bool RenderScene::UpdateMeshProxy(const ProxyID proxyID, const MeshProxyDesc& desc)
{
    NFE_ASSERT(mMeshProxies.Has(proxyID), "Proxy does not exist");

    MeshProxy& proxy = mMeshProxies[proxyID];
    proxy.desc = desc;
    proxy.globalBox = desc.transform.TransformBox(desc.mesh->GetLocalBox());

    return true;
}

bool RenderScene::UpdateLightProxy(const ProxyID proxyID, const LightProxyDesc& desc)
{
    NFE_ASSERT(mLightProxies.Has(proxyID), "Proxy does not exist");

    LightProxy& proxy = mLightProxies[proxyID];
    proxy.desc = desc;

    return true;
}

void RenderScene::DeleteMeshProxy(const ProxyID proxyID)
{
    mMeshProxies.Remove(proxyID);
}

void RenderScene::DeleteLightProxy(const ProxyID proxyID)
{
    mLightProxies.Remove(proxyID);
}

void RenderScene::UpdateLights()
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

void RenderScene::Update(float dt)
{
    UpdateLights();
}

} // namespace Renderer
} // namespace NFE

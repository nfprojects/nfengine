/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of light component class.
 */

#include "PCH.hpp"
#include "ComponentLight.hpp"
#include "../Entity.hpp"
#include "../SceneManager.hpp"
#include "../Systems/RendererSystem.hpp"
#include "Resources/ResourcesManager.hpp"
#include "Engine.hpp"
#include "Renderer/RenderScene.hpp"
#include "Renderer/LightsRenderer.hpp"

#include "nfCommon/Utils/InputStream.hpp"
#include "nfCommon/Utils/OutputStream.hpp"
#include "nfCommon/Logger/Logger.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::LightComponent)
    NFE_CLASS_PARENT(NFE::Scene::IComponent)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

using namespace Math;
using namespace Resource;
using namespace Renderer;

LightComponent::LightComponent()
    : mColor(Vector(1.0f, 1.0f, 1.0f, 1.0f))
    , mLightMap(nullptr)
    , mLightType(LightType::Omni)
    , mShadowMapResolution(0)
    , mRenderingProxy(INVALID_RENDER_PROXY)
{
}

LightComponent::~LightComponent()
{
    Release();
}

Math::Box LightComponent::GetBoundingBox() const
{
    // TODO
    return Math::Box();
}

void LightComponent::OnAttach()
{
    CreateRenderingProxy(false);
}

void LightComponent::OnDetach()
{
    if (mRenderingProxy != INVALID_RENDER_PROXY)
    {
        RenderScene* renderScene = GetScene()->GetSystem<RendererSystem>()->GetRenderScene();
        renderScene->DeleteLightProxy(mRenderingProxy);
        mRenderingProxy = INVALID_RENDER_PROXY;
    }
}

void LightComponent::OnUpdate()
{
    // update proxy
    CreateRenderingProxy(true);
}


void LightComponent::SetColor(const Float3& color)
{
    mColor = Vector(color);

    // update proxy
    CreateRenderingProxy(true);
}

void LightComponent::Release()
{
    if (mLightMap)
    {
        mLightMap->DelRef();
        mLightMap = nullptr;
    }
}

void LightComponent::SetOmniLight(const OmniLightDesc& desc)
{
    mOmniLight = desc;

    if (mLightType != LightType::Omni)
    {
        // update proxy
        CreateRenderingProxy(true);
    }
}

void LightComponent::SetSpotLight(const SpotLightDesc& desc)
{
    mSpotLight = desc;

    if (mLightType != LightType::Spot)
    {
        // update proxy
        CreateRenderingProxy(true);
    }
}

void LightComponent::SetDirLight(const DirLightDesc& desc)
{
    mDirLight = desc;

    if (mLightType != LightType::Dir)
    {
        // update proxy
        CreateRenderingProxy(true);
    }
}

bool LightComponent::SetShadowMap(uint16 resolution)
{
    // TODO update rendering proxy


    // TODO move this stuff to renderer
    /*
    if (!mShadowMap && resolution > 0)
    {
        mShadowMap.reset(new ShadowMap);
        if (mShadowMap == nullptr)
        {
            LOG_ERROR("Failed to create shadowmap object.");
            return false;
        }
    }
    else if (mShadowMap && resolution == 0)
    {
        mShadowMap.reset();
        return true;
    }
    else if (!mShadowMap && resolution == 0)
        return true;

    bool ret;
    switch (mLightType)
    {
    case LightType::Omni:
        ret = mShadowMap->Resize(resolution, ShadowMap::Type::Cube);
        break;
    case LightType::Spot:
        ret = mShadowMap->Resize(resolution, ShadowMap::Type::Flat);
        break;
    case LightType::Dir:
        ret = mShadowMap->Resize(resolution, ShadowMap::Type::Cascaded, mDirLight.splits);
        break;
    default:
        LOG_ERROR("Invalid light type");
        return false;
    }

    return ret;
    */

    return true;
}

void LightComponent::OnLightMapTextureLoaded()
{
    // TODO update rendering proxy

    // TODO move this stuff to renderer
    /*
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    std::recursive_mutex& renderingMutex = Engine::GetInstance()->GetRenderingMutex();
    std::unique_lock<std::recursive_mutex> lock(renderingMutex);

    mLightMapBindingInstance = renderer->GetDevice()->CreateResourceBindingInstance(
        LightsRenderer::Get()->GetLightMapBindingSet());
    if (!mLightMapBindingInstance)
    {
        LOG_ERROR("Failed to create light map's binding instance");
        return;
    }

    if (!mLightMapBindingInstance->WriteTextureView(0, mLightMap->GetRendererTexture()))
    {
        LOG_ERROR("Failed to write light map's binding instance");
        return;
    }
    */
}

void LightComponent::SetLightMap(const char* name)
{
    if (!name || (strnlen_s(name, RES_NAME_MAX_LENGTH) == 0))
    {
        if (mLightMap)
        {
            mLightMap->DelRef();
            mLightMap = 0;
        }
        return;
    }

    ResManager* rm = Engine::GetInstance()->GetResManager();
    Texture* newTexture = static_cast<Texture*>(rm->GetResource(name, ResourceType::Texture));

    if (newTexture != mLightMap)
    {
        if (mLightMap)
        {
            mLightMap->DelRef();
        }

        mLightMap = newTexture;
        if (mLightMap)
        {
            mLightMap->AddPostLoadCallback(std::bind(&LightComponent::OnLightMapTextureLoaded, this));
            mLightMap->AddRef();
        }
    }
}

void LightComponent::CreateRenderingProxy(bool update)
{
    Entity* entity = GetEntity();
    if (!GetEntity())
        return;

    SceneManager* scene = entity->GetScene();
    NFE_ASSERT(scene, "Entity not attached to a scene");

    // create the new proxy
    LightProxyDesc proxy;
    proxy.transform = GetEntity()->GetGlobalTransform().ToMatrix();
    proxy.color = mColor;
    proxy.shadowMapSize = mShadowMapResolution;

    if (mLightType == LightType::Omni)
    {
        proxy.type = LightProxyType::Omni;
        proxy.omni.radius = mOmniLight.radius;
    }
    else if (mLightType == LightType::Spot)
    {
        proxy.type = LightProxyType::Spot;
        proxy.spot.nearDistance = mSpotLight.nearDist;
        proxy.spot.farDistance = mSpotLight.farDist;
        proxy.spot.cutoffAngle = mSpotLight.cutoff;
    }


    RenderScene* renderScene = scene->GetSystem<RendererSystem>()->GetRenderScene();

    if (update)
    {
        NFE_ASSERT(mRenderingProxy != INVALID_RENDER_PROXY, "Trying to update invalid light render proxy");

        renderScene->UpdateLightProxy(mRenderingProxy, proxy);
    }
    else
    {
        mRenderingProxy = renderScene->CreateLightProxy(proxy);
    }
}

} // namespace Scene
} // namespace NFE

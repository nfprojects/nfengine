/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of light component class.
 */

#include "PCH.hpp"
#include "ComponentLight.hpp"
#include "Resources/ResourcesManager.hpp"
#include "Engine.hpp"
#include "Renderer/LightsRenderer.hpp"

#include "nfCommon/InputStream.hpp"
#include "nfCommon/OutputStream.hpp"
#include "nfCommon/Logger.hpp"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Resource;
using namespace Renderer;

LightComponent::LightComponent()
    : mColor(Vector(1.0f, 1.0f, 1.0f, 1.0f))
    , mLightMap(nullptr)
    , mLightType(LightType::Omni)
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

void LightComponent::OnAttach(Entity* entity)
{
    UNUSED(entity);
}

void LightComponent::OnDetach()
{

}

void LightComponent::SetColor(const Float3& color)
{
    mColor = color;
}

void LightComponent::Release()
{
    if (mLightMap)
    {
        mLightMap->DelRef();
        mLightMap = nullptr;
    }
}

void LightComponent::SetOmniLight(const OmniLightDesc* pDesc)
{
    mOmniLight = *pDesc;

    if (mLightType != LightType::Omni)
    {
        // TODO update render proxy
    }
}

void LightComponent::SetSpotLight(const SpotLightDesc* pDesc)
{
    mSpotLight = *pDesc;
    if (mLightType != LightType::Spot)
    {
        // TODO update render proxy
    }
}

void LightComponent::SetDirLight(const DirLightDesc* pDesc)
{
    mDirLight = *pDesc;

    if (mLightType != LightType::Dir)
    {
        // TODO update render proxy
    }
}

bool LightComponent::SetShadowMap(uint32 resolution)
{
    // TODO update render proxy
    return false;

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
}

void LightComponent::OnLightMapTextureLoaded()
{
    // TODO create rendering proxy

    // TODO move this stuff to renderer

    /*
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    std::recursive_mutex& renderingMutex = Engine::GetInstance()->GetRenderingMutex();
    std::unique_lock<std::recursive_mutex> lock(renderingMutex);

    mLightMapBindingInstance.reset(renderer->GetDevice()->CreateResourceBindingInstance(
        LightsRenderer::Get()->GetLightMapBindingSet().get()));
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

} // namespace Scene
} // namespace NFE

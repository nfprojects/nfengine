/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of light component class.
 */

#include "PCH.hpp"
#include "LightComponent.hpp"
#include "Resources/ResourcesManager.hpp"
#include "Engine.hpp"
#include "Renderer/LightsRenderer.hpp"

#include "nfCommon/Utils/InputStream.hpp"
#include "nfCommon/Utils/OutputStream.hpp"
#include "nfCommon/Logger/Logger.hpp"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Resource;
using namespace Renderer;

NFE_REGISTER_COMPONENT(LightComponent);

LightComponent::LightComponent()
    : mColor(Vector(1.0f, 1.0f, 1.0f, 1.0f))
    , mShadowMap(nullptr)
    , mLightMap(nullptr)
    , mLightType(LightType::Unknown)
{
}

LightComponent::~LightComponent()
{
    Release();
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
        Release();

        mLightType = LightType::Omni;

        if (mShadowMap)
            mShadowMap->Resize(mShadowMap->GetSize(), ShadowMap::Type::Cube);
    }
}

void LightComponent::SetSpotLight(const SpotLightDesc* pDesc)
{
    mSpotLight = *pDesc;
    if (mLightType != LightType::Spot)
    {
        Release();

        mLightType = LightType::Spot;

        if (mShadowMap)
            mShadowMap->Resize(mShadowMap->GetSize(), ShadowMap::Type::Flat);
    }
}

void LightComponent::SetDirLight(const DirLightDesc* pDesc)
{
    mDirLight = *pDesc;

    if (mLightType != LightType::Dir)
    {
        Release();

        mLightType = LightType::Dir;

        if (mShadowMap)
            mShadowMap->Resize(mShadowMap->GetSize(), ShadowMap::Type::Cascaded);
    }
}

bool LightComponent::SetShadowMap(uint32 resolution)
{
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
}

bool LightComponent::HasShadowMap() const
{
    if (mShadowMap == nullptr)
        return false;
    return (mShadowMap->GetSize() > 0);
}

void LightComponent::OnLightMapTextureLoaded()
{
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
}

void LightComponent::SetLightMap(const char* pName)
{
    if ((pName == NULL) || (strnlen_s(pName, RES_NAME_MAX_LENGTH) == 0))
    {
        if (mLightMap)
        {
            mLightMap->DelRef();
            mLightMap = 0;
        }
        return;
    }

    ResManager* rm = Engine::GetInstance()->GetResManager();
    Texture* newTexture = static_cast<Texture*>(rm->GetResource(pName, ResourceType::Texture));

    if (newTexture != mLightMap)
    {
        if (mLightMap)
            mLightMap->DelRef();

        mLightMap = newTexture;
        if (mLightMap)
        {
            mLightMap->AddPostLoadCallback(std::bind(&LightComponent::OnLightMapTextureLoaded, this));
            mLightMap->AddRef();
        }
    }
}

bool LightComponent::CanBeTiled()
{
    if (mLightType == LightType::Omni)
        return mShadowMap != NULL;

    return false;
}

} // namespace Scene
} // namespace NFE

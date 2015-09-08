/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of light component class.
 */

#include "../PCH.hpp"
#include "LightComponent.hpp"
#include "../ResourcesManager.hpp"

#include "../nfCommon/InputStream.hpp"
#include "../nfCommon/OutputStream.hpp"
#include "../nfCommon/Logger.hpp"
#include "../Engine.hpp"

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

LightComponent::LightComponent(const LightComponent& other)
{
    mLightType = other.mLightType;
    mColor = other.mColor;

    switch (mLightType)
    {
    case LightType::Omni:
        mOmniLight = other.mOmniLight;
        break;
    case LightType::Spot:
        mSpotLight = other.mSpotLight;
        break;
    case LightType::Dir:
        mDirLight = other.mDirLight;
        break;
    default:
        LOG_ERROR("Invalid light type");
    }

    mShadowMap = nullptr;
    mLightMap = other.mLightMap;
    if (mLightMap)
        mLightMap->AddRef();
}

void LightComponent::SetColor(const Float3& color)
{
    mColor = color;
}

void LightComponent::Release()
{
    if (mShadowMap != nullptr)
    {
        mShadowMap->Release();
        mShadowMap = nullptr;
    }

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

Result LightComponent::SetShadowMap(uint32 resolution)
{
    if (!mShadowMap && resolution > 0)
    {
        mShadowMap = nullptr; // TODO
        if (mShadowMap == 0)
        {
            LOG_ERROR("Failed to create shadowmap object.");
            return Result::AllocationError;
        }
    }
    else if (mShadowMap && resolution == 0)
    {
        delete mShadowMap;
        mShadowMap = nullptr;
        return Result::OK;
    }
    else if (!mShadowMap && resolution == 0)
        return Result::OK;

    switch (mLightType)
    {
    case LightType::Omni:
        mShadowMap->Resize(resolution, ShadowMap::Type::Cube);
        break;
    case LightType::Spot:
        mShadowMap->Resize(resolution, ShadowMap::Type::Flat);
        break;
    case LightType::Dir:
        mShadowMap->Resize(resolution, ShadowMap::Type::Cascaded, mDirLight.splits);
        break;
    default:
        LOG_ERROR("Invalid light type");
    }

    return Result::OK;
}

bool LightComponent::HasShadowMap() const
{
    if (mShadowMap == nullptr)
        return false;
    return (mShadowMap->GetSize() > 0);
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
            mLightMap->AddRef();
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

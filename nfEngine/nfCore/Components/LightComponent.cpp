/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of light component class.
 */

#include "../PCH.hpp"
#include "LightComponent.hpp"
#include "../Globals.hpp"
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
{
    mLightType = LightType::Unknown;
    mShadowMap = nullptr;

    mColor = Vector(1.0f, 1.0f, 1.0f, 1.0f);
    mDrawShadow = false;
    mLightMap = 0;
}

LightComponent::~LightComponent()
{
    Release();

    if (mLightMap)
    {
        mLightMap->DelRef();
        mLightMap = nullptr;
    }
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

    Texture* pNewTexture = ENGINE_GET_TEXTURE(pName);

    if (pNewTexture != mLightMap)
    {
        if (mLightMap) mLightMap->DelRef();
        pNewTexture->AddRef();
        mLightMap = pNewTexture;
    }
}

void LightComponent::Update(CameraComponent* pCamera)
{
    /*
    if (mLightType == LightType::Spot)
    {
        //set perspective projection
        Perspective perspectiveDesc;
        perspectiveDesc.aspectRatio = 1.0f;
        perspectiveDesc.farDist = 100.0f;
        perspectiveDesc.nearDist = 0.1f;
        perspectiveDesc.FoV = mSpotLight.cutoff;
        mCameras[0]->SetPerspective(&perspectiveDesc);

        //set view the same as light
        //  mCameras[0]->mOwner->mMatrix = mOwner->mMatrix;

        //update camera
        mCameras[0]->Update(0.0f);
    }

    if (mShadowMap && mShadowMap->GetSize() > 0)
    {
        if (mLightType == LightType::Omni)
        {
            // +X
            mCameras[0]->mParentOffset.r[0] = Vector(0, 0, -1);
            mCameras[0]->mParentOffset.r[1] = Vector(0, 1, 0);
            mCameras[0]->mParentOffset.r[2] = Vector(1, 0, 0);

            // -X
            mCameras[1]->mParentOffset.r[0] = Vector(0, 0, 1);
            mCameras[1]->mParentOffset.r[1] = Vector(0, 1, 0);
            mCameras[1]->mParentOffset.r[2] = Vector(-1, 0, 0);

            // +Y
            mCameras[2]->mParentOffset.r[0] = Vector(1, 0, 0);
            mCameras[2]->mParentOffset.r[1] = Vector(0, 0, -1);
            mCameras[2]->mParentOffset.r[2] = Vector(0, 1, 0);

            // -Y
            mCameras[3]->mParentOffset.r[0] = Vector(1, 0, 0);
            mCameras[3]->mParentOffset.r[1] = Vector(0, 0, 1);
            mCameras[3]->mParentOffset.r[2] = Vector(0, -1, 0);


            // +Z
            mCameras[4]->mParentOffset.r[0] = Vector(1, 0, 0);
            mCameras[4]->mParentOffset.r[1] = Vector(0, 1, 0);
            mCameras[4]->mParentOffset.r[2] = Vector(0, 0, 1);

            // -Z
            mCameras[5]->mParentOffset.r[0] = Vector(-1, 0, 0);
            mCameras[5]->mParentOffset.r[1] = Vector(0, 1, 0);
            mCameras[5]->mParentOffset.r[2] = Vector(0, 0, -1);

            // set perspective projection
            Perspective perspectiveDesc;
            perspectiveDesc.aspectRatio = 1.0f;
            perspectiveDesc.farDist = mOmniLight.radius;
            perspectiveDesc.nearDist = mOmniLight.radius * 0.001f;
            perspectiveDesc.FoV = NFE_MATH_PI * 0.5f;

            //update camera
            for (int i = 0; i < 6; i++)
            {
                mCameras[i]->SetPerspective(&perspectiveDesc);
                //mCameras[i]->mOwner->SetPosition(mOwner->GetPosition());
                mCameras[i]->OnUpdate(0.0f);
            }
        }

        if (mLightType == LightType::Dir)
        {
            Frustum frustum;

            float camNear = 5.0f;
            float camFar = mDirLight.farDist;
            const float blend = 0.2f;
            float linLogFactor = 0.8f;

            float cascadeCount = (float)mDirLight.splits;
            for (int32 i = 0; i < mDirLight.splits - 1; i++)
            {
                float logFactor = camNear * powf(camFar / camNear, ((float)i + blend) / cascadeCount);
                float linFactor = camNear + (((float)i + blend) / cascadeCount) * (camFar - camNear);

                mCascadeRanges[i].f[1] = Lerp(linFactor, logFactor, linLogFactor);
            }

            for (int32 i = 1; i < mDirLight.splits; i++)
            {
                float logFactor = camNear * powf(camFar / camNear, ((float)i - 1.0f - blend) / cascadeCount);
                float linFactor = camNear + (((float)i - 1.0f - blend) / cascadeCount) * (camFar - camNear);

                mCascadeRanges[i].f[0] = Lerp(linFactor, logFactor, linLogFactor);
            }

            mCascadeRanges[mDirLight.splits - 1].f[1] = camFar;
            mCascadeRanges[0].f[0] = 0.0f;


            for (int i = 0; i < mDirLight.splits; i++)
            {
                pCamera->SplitFrustum(mCascadeRanges[i].f[0], mCascadeRanges[i].f[1], &frustum);
                Sphere boundingSphere(frustum.verticies[6], frustum.verticies[7],
                                      frustum.verticies[5], frustum.verticies[3]);

                Ortho ortho;
                ortho.right =    boundingSphere.SupportVertex(   mOwner->mMatrix.GetRow(0));
                ortho.left =     -boundingSphere.SupportVertex(  -mOwner->mMatrix.GetRow(0));
                ortho.top =      boundingSphere.SupportVertex(   mOwner->mMatrix.GetRow(1));
                ortho.bottom =  -boundingSphere.SupportVertex(  -mOwner->mMatrix.GetRow(1));
                ortho.farDist =  boundingSphere.SupportVertex(   mOwner->mMatrix.GetRow(2));
                ortho.nearDist = -boundingSphere.SupportVertex(  -mOwner->mMatrix.GetRow(2)) - 50.0f;


                float worldUnitsPerTexel = 2.0f * boundingSphere.r / 1024.0f;
                ortho.left = Quantize(ortho.left, worldUnitsPerTexel);
                ortho.right = Quantize(ortho.right, worldUnitsPerTexel);
                ortho.bottom = Quantize(ortho.bottom, worldUnitsPerTexel);
                ortho.top = Quantize(ortho.top, worldUnitsPerTexel);
                mCameras[i]->SetOrtho(&ortho);


                Orientation orient;
                mOwner->GetOrientation(&orient);
                mCameras[i]->mOwner->SetOrientation(&orient);
                mCameras[i]->mOwner->SetPosition(Vector());
                mCameras[i]->OnUpdate(0.0f);
            }
        }
    }
    */
}

bool LightComponent::CanBeTiled()
{
    if (mLightType == LightType::Omni)
        return mShadowMap != NULL;

    return false;
}

} // namespace Scene
} // namespace NFE

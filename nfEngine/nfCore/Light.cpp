/**
    NFEngine project

    \file   Light.cpp
    \brief  Definition of light component class.
*/

#include "stdafx.h"
#include "Globals.h"
#include "Light.h"
#include "Entity.h"
#include "Renderer.h"
#include "ResourcesManager.h"
#include "../nfCommon/InputStream.h"
#include "../nfCommon/OutputStream.h"
#include "../nfCommon/Logger.h"
#include "Engine.h"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Resource;
using namespace Render;

// TODO: this should be in math library...
int CalculateEquations(float M[4][3], float* pResult1, float* pResult2, float* pResult3)
{
    float W = M[0][0] * M[1][1] * M[2][2] +
              M[1][0] * M[2][1] * M[0][2] +
              M[2][0] * M[0][1] * M[1][2] -
              M[2][0] * M[1][1] * M[0][2] -
              M[1][0] * M[0][1] * M[2][2] -
              M[0][0] * M[2][1] * M[1][2];

    if ((W < 0.000001f) && (W > -0.000001f))
    {
        return 1;
    }

    float Wx = M[3][0] * M[1][1] * M[2][2] +
               M[1][0] * M[2][1] * M[3][2] +
               M[2][0] * M[3][1] * M[1][2] -
               M[2][0] * M[1][1] * M[3][2] -
               M[1][0] * M[3][1] * M[2][2] -
               M[3][0] * M[2][1] * M[1][2];

    float Wy = M[0][0] * M[3][1] * M[2][2] +
               M[3][0] * M[2][1] * M[0][2] +
               M[2][0] * M[0][1] * M[3][2] -
               M[2][0] * M[3][1] * M[0][2] -
               M[3][0] * M[0][1] * M[2][2] -
               M[0][0] * M[2][1] * M[3][2];

    float Wz = M[0][0] * M[1][1] * M[3][2] +
               M[1][0] * M[3][1] * M[0][2] +
               M[3][0] * M[0][1] * M[1][2] -
               M[3][0] * M[1][1] * M[0][2] -
               M[1][0] * M[0][1] * M[3][2] -
               M[0][0] * M[3][1] * M[1][2];

    *pResult1 = Wx / W;
    *pResult2 = Wy / W;
    *pResult3 = Wz / W;

    return 0;
}

// TODO: this should be in math library...
Sphere MakeSphereFromPoints(const Vector& P1, const Vector& P2, const Vector& P3, const Vector& P4)
{
    float matrix[4][3];
    Vector sphereCenter;

    matrix[0][0] = 2.0f * (P2.f[0] - P1.f[0]);
    matrix[1][0] = 2.0f * (P2.f[1] - P1.f[1]);
    matrix[2][0] = 2.0f * (P2.f[2] - P1.f[2]);

    matrix[0][1] = 2.0f * (P3.f[0] - P1.f[0]);
    matrix[1][1] = 2.0f * (P3.f[1] - P1.f[1]);
    matrix[2][1] = 2.0f * (P3.f[2] - P1.f[2]);

    matrix[0][2] = 2.0f * (P4.f[0] - P1.f[0]);
    matrix[1][2] = 2.0f * (P4.f[1] - P1.f[1]);
    matrix[2][2] = 2.0f * (P4.f[2] - P1.f[2]);

    float Tmp = VectorDot3f(P1, P1);
    matrix[3][0] = VectorDot3f(P2, P2) - Tmp;
    matrix[3][1] = VectorDot3f(P3, P3) - Tmp;
    matrix[3][2] = VectorDot3f(P4, P4) - Tmp;

    Sphere result;
    CalculateEquations(matrix, &result.origin.f[0], &result.origin.f[1], &result.origin.f[2]);
    result.r = VectorLength3(result.origin - P1).f[0];
    return result;
}


LightComponent::LightComponent(Entity* pParent) : Component(pParent)
{
    mType = ComponentType::Light;

    mLightType = LightType::Unknown;
    mShadowMap = nullptr;

    mColor = Vector(1.0f, 1.0f, 1.0f, 1.0f);
    mUpdateShadowmap = false;
    mDrawShadow = false;
    mLightMap = 0;
}

LightComponent::~LightComponent()
{
    Release();

    if (mShadowMap)
    {
        delete mShadowMap;
        mShadowMap = 0;
    }

    if (mLightMap)
    {
        mLightMap->DelRef();
        mLightMap = 0;
    }
}

void LightComponent::SetColor(const Float3& color)
{
    mColor = color;
}

void LightComponent::Release()
{
    if (mShadowMap != 0)
    {
        mShadowMap->Release();
        mShadowMap = 0;
    }

    if (mCameras.size())
    {
        //for (auto pCamera : mCameras)
        //{
        //  mOwner->RemoveComponent(pCamera);
        //}

        mCameras.clear();
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
            mShadowMap->Resize(mShadowMap->GetSize(), IShadowMap::Type::Cube);

        for (int i = 0; i < 6; i++)
        {
            Camera* pCamera = new Camera(mOwner);
            pCamera->mVirtual = true;
            mCameras.push_back(pCamera);
        }
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
            mShadowMap->Resize(mShadowMap->GetSize(), IShadowMap::Type::Flat);

        {
            Camera* pCamera = new Camera(mOwner);
            pCamera->mVirtual = true;
            mCameras.push_back(pCamera);
        }
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
            mShadowMap->Resize(mShadowMap->GetSize(), IShadowMap::Type::Cascaded);

        for (int i = 0; i < mDirLight.splits; i++)
        {
            Camera* pCamera = new Camera(mOwner);
            pCamera->mVirtual = true;
            mCameras.push_back(pCamera);
        }
    }
}

Result LightComponent::SetShadowMap(uint32 resolution)
{
    if (!mShadowMap && resolution > 0)
    {
        mShadowMap = g_pRenderer->CreateShadowMap();
        if (mShadowMap == 0)
        {
            LOG_ERROR("Failed to create shadowmap object.");
            return Result::AllocationError;
        }
    }
    else if (mShadowMap && resolution == 0)
    {
        delete mShadowMap;
        mShadowMap = 0;
        return Result::OK;
    }
    else if (!mShadowMap && resolution == 0)
        return Result::OK;

    switch (mLightType)
    {
        case LightType::Omni:
            mShadowMap->Resize(resolution, IShadowMap::Type::Cube);
            break;

        case LightType::Spot:
            mShadowMap->Resize(resolution, IShadowMap::Type::Flat);
            break;

        case LightType::Dir:
            mShadowMap->Resize(resolution, IShadowMap::Type::Cascaded, mDirLight.splits);
            break;
    }

    return Result::OK;
}

bool LightComponent::HasShadowMap() const
{
    if (mShadowMap == 0) return false;
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


float Lerp(float a, float b, float x)
{
    return a + x * (b - a);
}


float Quantize(float x, float step)
{
    float tmp = x / step;
    tmp = floor(tmp);
    return tmp * step;

    /*
    float r = fmodf(x, step);

    if (x < 0.0f)
        return x + r - step;

    return x - r + step;*/
}

void LightComponent::Update(Camera* pCamera)
{
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
        mCameras[0]->OnUpdate(0.0f);
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

            /*
            //set orientation for each face
            XOrientation orientation;

            //X+
            orientation.z = Vector(1, 0, 0);
            orientation.y = Vector(0, 1, 0);
            mCameras[0]->mOwner->SetOrientation(&orientation);

            //X-
            orientation.z = Vector(-1, 0, 0);
            orientation.y = Vector(0, 1, 0);
            mCameras[1]->mOwner->SetOrientation(&orientation);

            //Y+
            orientation.z = Vector(0, 1, 0);
            orientation.y = Vector(0, 0, -1);
            mCameras[2]->mOwner->SetOrientation(&orientation);

            //Y-
            orientation.z = Vector(0, -1, 0);
            orientation.y = Vector(0, 0, 1);
            mCameras[3]->mOwner->SetOrientation(&orientation);

            //Z+
            orientation.z = Vector(0, 0, 1);
            orientation.y = Vector(0, 1, 0);
            mCameras[4]->mOwner->SetOrientation(&orientation);

            //Z-
            orientation.z = Vector(0, 0, -1);
            orientation.y = Vector(0, 1, 0);
            mCameras[5]->mOwner->SetOrientation(&orientation);
            */

            //set perspective projection
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
                Sphere boundingSphere = MakeSphereFromPoints(frustum.verticies[6], frustum.verticies[7],
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


                XOrientation orient;
                mOwner->GetOrientation(&orient);
                mCameras[i]->mOwner->SetOrientation(&orient);
                mCameras[i]->mOwner->SetPosition(Vector());




                /*
                Ortho ortho;
                ortho.mRight =     frustum.SupportVertex(  mMatrix.GetRow(0));
                ortho.mLeft =      -frustum.SupportVertex( -mMatrix.GetRow(0));
                ortho.mTop =       frustum.SupportVertex(  mMatrix.GetRow(1));
                ortho.mBottom =    -frustum.SupportVertex( -mMatrix.GetRow(1));
                ortho.farDist =       frustum.SupportVertex(  mMatrix.GetRow(2));
                ortho.nearDist =      -frustum.SupportVertex( -mMatrix.GetRow(2)) - 50.0f;
                mCameras[i].SetOrtho(&ortho);

                XOrientation orient;
                GetOrientation(&orient);
                mCameras[i].SetOrientation(&orient);
                mCameras[i].SetPosition(Vector());
                */

                mCameras[i]->OnUpdate(0.0f);
            }
        }
    }
}

void LightComponent::OnRender(IRenderContext* pCtx)
{
    IShadowMap* pShadowMap = mDrawShadow ? mShadowMap : nullptr;

    if (mLightType == LightType::Omni)
    {
        g_pLightRenderer->DrawOmniLight(pCtx, mOwner->GetPosition(), mOmniLight.radius, mColor,
                                        pShadowMap);
    }

    if (mLightType == LightType::Spot)
    {
        SpotLightProperties prop;
        prop.color = mColor;
        prop.position = mOwner->mMatrix.r[3];
        prop.direction = mOwner->mMatrix.r[2];
        prop.farDist.Set(mSpotLight.farDist);
        prop.viewProjMatrix = mCameras[0]->mViewMatrix * mCameras[0]->mProjMatrix;
        prop.viewMatrix = mCameras[0]->mViewMatrix;

        if (pShadowMap)
            prop.shadowMapResInv = 1.0f / (float)mShadowMap->GetSize();
        else
            prop.shadowMapResInv = 0.0f;


        g_pLightRenderer->DrawSpotLight(pCtx, prop, mCameras[0]->mFrustum, pShadowMap,
                                        mLightMap ? mLightMap->GetRendererTexture() : NULL);
    }

    if (mLightType == LightType::Dir)
    {
        DirLightProperties prop;
        prop.color = mColor;
        prop.direction = mOwner->mMatrix.GetRow(2);
        prop.cascadesCount[0] = mDirLight.splits;

        for (int i = 0; i < mDirLight.splits; i++)
        {
            prop.splitDistance[i] = mCascadeRanges[i];
            prop.viewProjMatrix[i] = mCameras[i]->mViewMatrix * mCameras[i]->mProjMatrix;
        }

        g_pLightRenderer->DrawDirLight(pCtx, prop, pShadowMap);
    }
}


void LightComponent::OnRenderDebug(IRenderContext* pCtx)
{
    if (mLightType == LightType::Omni)
    {
        Box box;
        box.min = mOwner->GetPosition() - VectorSplat(mOmniLight.radius);
        box.max = mOwner->GetPosition() + VectorSplat(mOmniLight.radius);
        g_pDebugRenderer->DrawBox(pCtx, box, 0xFF00FFFF);
    }

    if (mLightType == LightType::Spot)
    {
        g_pDebugRenderer->DrawFrustum(pCtx, mCameras[0]->mFrustum, 0xFF00FFFF);

        Float3 o, x, y, z;
        VectorStore(mOwner->GetPosition(), &o);
        VectorStore(mOwner->GetPosition() + mOwner->mMatrix.GetRow(0), &x);
        VectorStore(mOwner->GetPosition() + mOwner->mMatrix.GetRow(1), &y);
        VectorStore(mOwner->GetPosition() + mOwner->mMatrix.GetRow(2), &z);

        g_pDebugRenderer->DrawLine(pCtx, o, x, 0xFF0000FF);
        g_pDebugRenderer->DrawLine(pCtx, o, y, 0xFF00FF00);
        g_pDebugRenderer->DrawLine(pCtx, o, z, 0xFFFF0000);
    }

    if (mLightType == LightType::Dir)
    {
        //for
        g_pDebugRenderer->DrawFrustum(pCtx, mCameras[0]->mFrustum, 0xFF00FFFF);
    }
}

int LightComponent::IntersectFrustum(const Frustum& frustum)
{
    if (mLightType == LightType::Spot)
    {
        return IntersectFrustumFrustum(frustum, mCameras[0]->mFrustum);
    }

    if (mLightType == LightType::Omni)
    {
        Vector radiusVec = Vector(mOmniLight.radius, mOmniLight.radius, mOmniLight.radius);

        Box lightBox; //must be changed to a sphere!!
        lightBox.min = mOwner->GetPosition() - radiusVec;
        lightBox.max = mOwner->GetPosition() + radiusVec;

        return IntersectBoxFrustum(lightBox, frustum);
    }

    return 1;
}

void LightComponent::CheckShadowVisibility(const Vector& camPos)
{
    if (!HasShadowMap())
    {
        mDrawShadow = false;
        return;
    }


    if (mLightType == LightType::Spot)
    {
        if (VectorLength3(camPos - mOwner->GetPosition()).f[0] > mSpotLight.maxShadowDistance)
        {
            mDrawShadow = false;
            return;
        }
    }

    if (mLightType == LightType::Omni)
    {
        if (VectorLength3(camPos - mOwner->GetPosition()).f[0] > mOmniLight.shadowFadeEnd)
        {
            mDrawShadow = false;
            return;
        }
    }

    mDrawShadow = true;
}

bool LightComponent::CanBeTiled()
{
    if (mLightType == LightType::Omni)
        return mShadowMap != NULL;

    return false;
}

Result LightComponent::Deserialize(Common::InputStream* pStream)
{
    LightDesc desc;
    if (pStream->Read(sizeof(desc), &desc) != sizeof(desc))
    {
        LOG_ERROR("Could not deserialize light component.");
        return Result::Error;
    }

    switch (desc.lightType)
    {
        case LightType::Omni:
            SetOmniLight(&desc.omni);
            break;

        case LightType::Spot:
            SetSpotLight(&desc.spot);
            break;

        case LightType::Dir:
            SetDirLight(&desc.dir);
            break;

        default:
            LOG_ERROR("Light component deserialization failed: invalid light type. Data could be corrupted.");
            Result::Error;
    };

    mColor = desc.color;
    SetShadowMap(desc.mShadowMapSize);
    SetLightMap(desc.mLightMapName);

    return Result::OK;
}

Result LightComponent::Serialize(Common::OutputStream* pStream) const
{
    LightDesc desc;
    ZeroMemory(&desc, sizeof(desc));

    switch (mLightType)
    {
        case LightType::Omni:
            desc.omni = mOmniLight;
            break;

        case LightType::Spot:
            desc.spot = mSpotLight;
            break;

        case LightType::Dir:
            desc.dir = mDirLight;
            break;
    };

    desc.lightType = (char)mLightType;
    VectorStore(mColor, &desc.color);
    desc.mShadowMapSize = (mShadowMap != nullptr) ? (uint16)mShadowMap->GetSize() : 0;

    if (mLightMap)
        strcpy_s(desc.mLightMapName, mLightMap->GetName());


    if (pStream->Write(&desc, sizeof(desc)) != sizeof(desc))
    {
        LOG_ERROR("Could not serialize light component.");
        return Result::Error;
    }

    return Result::OK;
}

} // namespace Scene
} // namespace NFE

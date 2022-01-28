/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of light component class.
 */

#include "PCH.hpp"
#include "ComponentLight.hpp"
#include "../Entity.hpp"
#include "../Scene.hpp"
#include "../Systems/RendererSystem.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::LightComponent)
    NFE_CLASS_PARENT(NFE::Scene::IComponent)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

using namespace Math;
using namespace Resource;

LightComponent::LightComponent()
    : mColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f))
    , mLightType(LightType::Omni)
    , mShadowMapResolution(0)
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
    //if (mRenderingProxy != INVALID_RENDER_PROXY)
    //{
    //    RenderScene* renderScene = GetScene().GetSystem<RendererSystem>()->GetRenderScene();
    //    renderScene->DeleteLightProxy(mRenderingProxy);
    //    mRenderingProxy = INVALID_RENDER_PROXY;
    //}
}

void LightComponent::OnUpdate()
{
    // update proxy
    CreateRenderingProxy(true);
}


void LightComponent::SetColor(const Vec3f& color)
{
    mColor = Vec4f(color);

    // update proxy
    CreateRenderingProxy(true);
}

void LightComponent::Release()
{
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

void LightComponent::CreateRenderingProxy(bool update)
{
    NFE_UNUSED(update);
    //Entity* entity = GetEntity();
    //if (!GetEntity())
    //    return;

    //Scene& scene = entity->GetScene();

    //// create the new proxy
    //LightProxyDesc proxy;
    //proxy.transform = GetEntity()->GetGlobalTransform().ToMatrix();
    //proxy.color = mColor;
    //proxy.shadowMapSize = mShadowMapResolution;

    //if (mLightType == LightType::Omni)
    //{
    //    proxy.type = LightProxyType::Omni;
    //    proxy.omni.radius = mOmniLight.radius;
    //}
    //else if (mLightType == LightType::Spot)
    //{
    //    proxy.type = LightProxyType::Spot;
    //    proxy.spot.nearDistance = mSpotLight.nearDist;
    //    proxy.spot.farDistance = mSpotLight.farDist;
    //    proxy.spot.cutoffAngle = mSpotLight.cutoff;
    //}


    //RenderScene* renderScene = scene.GetSystem<RendererSystem>()->GetRenderScene();

    //if (update)
    //{
    //    NFE_ASSERT(mRenderingProxy != INVALID_RENDER_PROXY, "Trying to update invalid light render proxy");

    //    renderScene->UpdateLightProxy(mRenderingProxy, proxy);
    //}
    //else
    //{
    //    mRenderingProxy = renderScene->CreateLightProxy(proxy);
    //}
}

} // namespace Scene
} // namespace NFE

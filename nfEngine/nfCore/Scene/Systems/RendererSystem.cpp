/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of renderer system.
 */

#include "PCH.hpp"
#include "RendererSystem.hpp"
#include "Renderer/RenderScene.hpp"

#include "nfCommon/System/Assertion.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::RendererSystem)
    NFE_CLASS_PARENT(NFE::Scene::ISystem)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

using namespace Common;
using namespace Math;

RendererSystem::RendererSystem(SceneManager* scene)
    : ISystem(scene)
{
    mRenderScene = MakeUniquePtr<Renderer::RenderScene>();
}

void RendererSystem::Update(float dt)
{
    if (!mRenderScene)
        return;

    mRenderScene->Update(dt);
}

} // namespace Scene
} // namespace NFE
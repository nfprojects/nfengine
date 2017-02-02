/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of renderer system.
 */

#include "PCH.hpp"
#include "PhysicsSystem.hpp"
#include "../../Physics/PhysicsSceneImpl.hpp"

#include "nfCommon/System/Assertion.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::PhysicsSystem)
    NFE_CLASS_PARENT(NFE::Scene::ISystem)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {


using namespace Common;
using namespace Math;
using namespace Physics;


PhysicsSystem::PhysicsSystem(SceneManager& scene)
    : ISystem(scene)
{
    mPhysicsScene = MakeUniquePtr<PhysicsScene>();
}

void PhysicsSystem::Update(const SystemUpdateContext& context)
{
    if (!mPhysicsScene)
        return;

    mPhysicsScene->Update(context.timeDelta);
}


} // namespace Scene
} // namespace NFE

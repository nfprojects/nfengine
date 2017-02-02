/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "System.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::ISystem)
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace Scene {

ISystem::ISystem(SceneManager& scene)
    : mScene(scene)
{ }

} // namespace Scene
} // namespace NFE

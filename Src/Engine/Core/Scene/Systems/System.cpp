/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "System.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::ISystem)
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace Scene {

ISystem::ISystem(Scene& scene)
    : mScene(scene)
{ }

} // namespace Scene
} // namespace NFE

/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of transform system.
 */

#include "../PCH.hpp"
#include "TransformSystem.hpp"
#include "../Globals.hpp"
#include "../Engine.hpp"
#include "../EntityManager.hpp"
#include "../Components/TransformComponent.hpp"

namespace NFE {
namespace Scene {

using namespace Math;

TransformSystem::TransformSystem(SceneManager* scene)
    : mScene(scene)
{
}

void TransformSystem::Update()
{

}

} // namespace Scene
} // namespace NFE

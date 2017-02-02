/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of script component class.
 */

#include "PCH.hpp"
#include "ComponentScript.hpp"
#include "Engine.hpp"
#include "../Entity.hpp"
#include "../SceneManager.hpp"

#include "nfCommon/Assertion.hpp"


namespace NFE {
namespace Scene {

Entity* NativeScript::GetEntity() const
{
    return nullptr;
}

ScriptComponent::ScriptComponent()
{
}

ScriptComponent::~ScriptComponent()
{
}

} // namespace Scene
} // namespace NFE

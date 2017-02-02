/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of input component class.
 */

#include "PCH.hpp"
#include "ComponentInput.hpp"
#include "Engine.hpp"
#include "../Entity.hpp"
#include "../SceneManager.hpp"


namespace NFE {
namespace Scene {

InputComponent::InputComponent()
{
}

InputComponent::~InputComponent()
{
}


bool InputComponent::OnEvent(const Input::Event& event)
{
    return true;
}

} // namespace Scene
} // namespace NFE
